#include "Actor/Boundary/Boundary.h"
#include "Actor/NPC/Bullet/FireController/NpcFierController.h"
#include "Actor/NPC/NPC.h"
#include "Actor/Station/Base/BaseStation.h"
#include "Frame/Frame.h"
#include "imgui.h"
#include "Actor/Player/Bullet/BasePlayerBullet.h"
#include "Actor/NPC/Bullet/NpcBullet.h"

// ルート関連
#include "Actor/NPC/Navigation/Route/RouteCollection.h"
#include "Actor/NPC/Navigation/Route/Route.h"
#include "Actor/Spline/Spline.h"

#include "3d/ViewProjection.h"
#include "random.h"

#include <algorithm>
#include <limits>
#include <cmath>

namespace{
inline bool NearerTo(const Vector3& p, NPC* a, NPC* b){
	const float da = (a->GetWorldPosition() - p).Length();
	const float db = (b->GetWorldPosition() - p).Length();
	return da < db;
}

inline float Smooth01(float x){
	x = std::clamp(x,0.0f,1.0f);
	return x * x * (3.0f - 2.0f * x); // smoothstep
}

inline Vector3 Lerp(const Vector3& a, const Vector3& b, float t){ return a + (b - a) * t; }

inline void YawPitchFromDir_LH(const Vector3& dir, float& outYaw, float& outPitch){
	const float eps = 1e-6f;
	const float lenXZ = std::sqrt(dir.x * dir.x + dir.z * dir.z);
	if (lenXZ < eps && std::fabs(dir.y) < eps){
		// ほぼゼロ方向：角度を変えない
		return;
	}
	// ヨー：+Zを基準にx-z平面の回転
	outYaw = std::atan2(dir.x,dir.z);
	// ピッチ：水平からの持ち上げ（下向きが+? -> LHなら -y が正）
	outPitch = std::atan2(-dir.y,(lenXZ > eps ? lenXZ : eps));
}
}

BaseStation::BaseStation(const std::string& name)
	: name_(name){}

BaseStation::~BaseStation() = default;

void BaseStation::Init(){
	// GlobalParameter
	globalParam_ = GlobalParameter::GetInstance();

	// メイングループ設定～同期
	globalParam_->CreateGroup(name_,true);
	globalParam_->ClearBindingsForGroup(name_);
	BindParms();
	globalParam_->SyncParamForGroup(name_);

	LoadData();

	// 初期座標適用
	baseTransform_.translation_ = initialPosition_;
	baseTransform_.scale_ = Vector3(1,1,1) * 10.0f;
	baseTransform_.UpdateMatrix();
	prevRoutePos_ = baseTransform_.translation_;

	// === AI 初期化 ===
	ai_.Initialize(this,unitDirector_);
	ai_.SetConfig(aiCfg_);

	// コライダー
	AABBCollider::Init();
	SetCollisionScale(Vector3(1,1,1) * 50.0f);

	// 初期スポーン
	StartupSpawn();

	hp_ = maxLife_;

	// 再計画タイマ（起動直後に走らせる）
	planTimer_ = 0.0f;

	routeInitialized_ = false;
}

void BaseStation::Update(){
	const float dt = Frame::DeltaTime();

	// ルートで動かす（動かした位置でAIや当たり判定を更新したいので最初に実行）
	MoveAlongRoute_(dt);

	BaseObject::Update();
	currentTime_ += dt;

	// AI 更新
	ai_.SetRivalCached(pRivalStation_);
	ai_.UpdateWeighted(dt,
					   /*self*/ hp_,maxLife_,
					   /*pos*/ baseTransform_.translation_,
					   /*rival*/ pRivalStation_,
					   /*H*/ homeThreatDebug_);

	// 役割再割当て
	planTimer_ -= dt;
	const float replan = ai_.Config().replanInterval; // GUI で編集可
	if (planTimer_ <= 0.0f){
		ReassignRoles();
		planTimer_ = (replan > 0.05f) ? replan : 0.5f;
	}

	// collider update
	cTransform_.translation_ = GetWorldPosition();
	cTransform_.UpdateMatrix();

	// 死んでいるnpcを回収
	CleanupSpawnedList();
}

void BaseStation::DrawDebug(const ViewProjection& vp){
	for (auto& h : spawned_){
		if (!h || !h->GetIsAlive()) continue;
		h->DebugDraw(vp);
	}
}

void BaseStation::ShowGui(){
	const std::string path = fileDirectory_;
	if (ImGui::CollapsingHeader(name_.c_str())){
		ImGui::PushID(name_.c_str());

		globalParam_->ParamLoadForImGui(name_,path);
		ImGui::SameLine();
		globalParam_->ParamSaveForImGui(name_,path);

		// Transform
		if (ImGui::TreeNode("Transform")){
			ImGui::DragFloat3("Scale",&baseTransform_.scale_.x,0.01f);
			ImGui::DragFloat3("Rotate",&baseTransform_.rotation_.x,0.01f);
			if (ImGui::DragFloat3("Translate",&baseTransform_.translation_.x,0.01f)){
				initialPosition_ = baseTransform_.GetWorldPos();
			}
			ImGui::TreePop();
		}

		// Station AI
		if (ImGui::TreeNode("Station AI")){
			// auto& c = ai_.Config();
			// ImGui::DragFloat("minAttack",&c.minAttack,0.01f,0.0f,1.0f);
			// ImGui::DragFloat("maxAttack",&c.maxAttack,0.01f,0.0f,1.0f);
			// ImGui::DragInt("minDefenders",&c.minDefenders,1,0,999);
			// ImGui::DragFloat("temperature",&c.temperature,0.01f,0.05f,3.0f);
			// ImGui::DragFloat("refAttackRange",&c.refAttackRange,1.0f,10.0f,5000.0f);
			// ImGui::DragFloat("dpsNormSelf",&c.dpsNormSelf,0.01f,0.05f,1.0f);
			// ImGui::DragFloat("dpsNormRival",&c.dpsNormRival,0.01f,0.05f,1.0f);
			// ImGui::DragFloat("smoothingSelf",&c.smoothingSelf,0.1f,0.5f,10.0f);
			// ImGui::DragFloat("smoothingRival",&c.smoothingRival,0.1f,0.5f,10.0f);
			// ImGui::DragFloat("replanInterval",&c.replanInterval,0.05f,0.1f,3.0f);
			// ImGui::DragFloat("ratioDeadband",&c.ratioDeadband,0.005f,0.0f,0.5f);

			ai_.ShowGui();
			ImGui::TreePop();
		}

		// Route
		if (ImGui::CollapsingHeader("Route")){
			ImGui::Checkbox("useRoute",&useRoute_);
			ImGui::DragFloat("routeLapTime",&routeLapTime_,0.1f,1.0f,120.0f);
			ImGui::SliderFloat("routePhase01",&routePhase01_,0.0f,1.0f);
			ImGui::DragInt("routeUnitIndex",&routeUnitIndex_,1,0,64);
			ImGui::Separator();
			ImGui::Checkbox("smoothWrap",&smoothWrap_);
			ImGui::DragFloat("wrapBlendDurationSec",&wrapBlendDurationSec_,0.05f,0.05f,5.0f);
			ImGui::Separator();
		}

		ImGui::PopID();
	}
}

void BaseStation::StartupSpawn(){
	const int target = std::clamp(initialSpawnCount_,0,maxConcurrentUnits_);
	const float R = initialSpawnDistanceFromThis_;
	const Vector3 c = GetWorldPosition();

	// ==== Yレンジ ====
	constexpr float kBoundaryY = 0.0f;
	constexpr float kMargin = 500.0f;

	float minY, maxY;
	if (c.y >= kBoundaryY){
		minY = kBoundaryY + kMargin; // +側
		maxY = c.y;
	}
	else{
		minY = c.y;
		maxY = kBoundaryY - kMargin; // -側
	}
	if (maxY < minY){ minY = maxY = c.y; }

	for (int i = 0; i < target; ++i){
		float x = c.x + Random::Range(-R,R);
		float z = c.z + Random::Range(-R,R);
		float y = Random::Range(minY,maxY);
		SpawnNPC(Vector3{x, y, z});
	}
}

void BaseStation::BindParms(){
	// 基本
	globalParam_->Bind(name_,"initialPosition",&initialPosition_);
	globalParam_->Bind(name_,"maxLife",&maxLife_);
	globalParam_->Bind(name_,"hp",&hp_);
	globalParam_->Bind(name_,"initialSpawnCount",&initialSpawnCount_);
	globalParam_->Bind(name_,"maxConcurrentUnits",&maxConcurrentUnits_);
	globalParam_->Bind(name_,"spawnInterbal",&spawnInterbal_);
	globalParam_->Bind(name_,"initialSpawnDistanceFromThis",&initialSpawnDistanceFromThis_);

	// ルート
	globalParam_->Bind(name_,"useRoute",&useRoute_);
	globalParam_->Bind(name_,"routeLapTime",&routeLapTime_);
	globalParam_->Bind(name_,"routePhase01",&routePhase01_);
	globalParam_->Bind(name_,"routeUnitIndex",&routeUnitIndex_);

	// 折り返し補間
	globalParam_->Bind(name_,"smoothWrap",&smoothWrap_);
	globalParam_->Bind(name_,"wrapBlendDurationSec",&wrapBlendDurationSec_);

}

void BaseStation::LoadData(){
	const std::string path = fileDirectory_;
	globalParam_->LoadFile(name_,path);
	globalParam_->SyncParamForGroup(name_);
}

void BaseStation::SaveData(){
	const std::string path = fileDirectory_;
	globalParam_->SaveFile(name_,path);
}

// accessor
void BaseStation::SetRivalStation(BaseStation* rival) noexcept{ pRivalStation_ = rival; }
BaseStation* BaseStation::GetRivalStation() const noexcept{ return pRivalStation_; }
void BaseStation::SetFaction(FactionType type) noexcept{ faction_ = type; }
FactionType BaseStation::GetFactionType() const noexcept{ return faction_; }

void BaseStation::SetRouteCollection(const RouteCollection* routeCollection) noexcept{
	pRouteCollection_ = routeCollection;
}

const RouteCollection* BaseStation::GetRouteCollection() const noexcept{ return pRouteCollection_; }

// === Route attach ===
void BaseStation::SetRoute(Route* route, int unitIndex) noexcept{
	route_ = route;
	routeUnitIndex_ = (std::max)(0,unitIndex);
	routeT01_ = std::fmod((std::max)(0.0f,routePhase01_),1.0f);

	// Outer 固定（ファイル名が Inner/Middle/Outer の想定で、読み込み順はソート済み）
	// → 最後の variant を Outer とみなす（必要なら名前判定に差し替え）
	routeActiveIndex_ = -1;
	if (route_){
		const int n = route_->GetUnitCount();
		if (n > 0){
			routeActiveIndex_ = n - 1; // Outer想定
		}
	}

	// 折り返し状態クリア
	wrapping_ = false;
	wrapTimer_ = 0.0f;

}

void BaseStation::AttachRouteFromCollection(RouteCollection* coll, RouteType type, int unitIndex) noexcept{
	if (!coll){
		route_ = nullptr;
		return;
	}
	Route* r = coll->GetRoute(type);
	SetRoute(r,unitIndex);
}

void BaseStation::AttachRouteByFaction(RouteCollection* coll, int unitIndex) noexcept{
	if (!coll){
		route_ = nullptr;
		return;
	}
	RouteType t = (faction_ == FactionType::Ally)
					  ? RouteType::AllyDifence
					  : RouteType::EnemyDirence;
	AttachRouteFromCollection(coll,t,unitIndex);
}

std::vector<NPC*> BaseStation::GetLiveNpcs() const{
	std::vector<NPC*> out;
	out.reserve(spawned_.size());
	for (auto& h : spawned_){ if (h && h->GetIsAlive()) out.push_back(h.get()); }
	return out;
}

std::vector<const NpcBullet*> BaseStation::GetLiveNpcBullets() const{
	std::vector<const NpcBullet*> out;
	out.reserve(spawned_.size());

	for (auto const& h : spawned_){
		if (!h) continue;
		NPC* npc = h.get();
		if (!npc->GetIsAlive()) continue;

		if (auto* fc = npc->GetFireController()){ fc->CollectAliveBullets(out); }
	}
	return out;
}

void BaseStation::CollectTargets(std::vector<const BaseObject*>& out) const{
	out.clear();
	if (pRivalStation_){
		for (NPC* npc : pRivalStation_->GetLiveNpcs()){
			if (!npc) continue;
			out.push_back(static_cast<const BaseObject*>(npc));
		}
		out.push_back(static_cast<const BaseObject*>(pRivalStation_));
	}
}

void BaseStation::OnCollisionEnter(BaseCollider* other){
	// 敵の基地にプレイヤー弾が当たったらダメージ
	if (faction_ == FactionType::Enemy){
		if (auto* bullet = dynamic_cast<BasePlayerBullet*>(other)){
			hp_ -= bullet->GetParameter().damage;
			if (hp_ < 0.0f) hp_ = 0.0f;
		}
		return;
	}

	// 味方の基地に敵弾が当たったらダメージ
	if (faction_ == FactionType::Ally){
		if (auto* bullet = dynamic_cast<NpcBullet*>(other)){
			hp_ -= bullet->GetDamage();
			if (hp_ < 0.0f) hp_ = 0.0f;
		}
	}
}

void BaseStation::ReassignRoles(){
	auto npcs = GetLiveNpcs();
	if (npcs.empty()) return;

	const auto out = ai_.GetLastOutput();
	const int N = static_cast<int>(npcs.size());

	int defenders = std::clamp(out.desiredDefenders,0,N);
	int attackers = std::clamp(out.desiredAttackers,0,N - defenders);

	// 安全策（攻撃対象が居るなら最低1体は攻撃へ）
	if (pRivalStation_ && N > 0 && attackers == 0){
		attackers = 1;
		defenders = (std::max)(0,N - attackers);
	}

	const Vector3 home = GetWorldPosition();
	const Vector3 rival = pRivalStation_ ? pRivalStation_->GetWorldPosition() : home;

	for (auto* npc : npcs) if (npc) npc->SetTargetProvider(this);

	// 1) 防衛：自陣近い順
	std::sort(npcs.begin(),npcs.end(),
			  [&](NPC* a, NPC* b){ return NearerTo(home,a,b); });

	for (int i = 0; i < defenders && i < (int)npcs.size(); ++i){
		auto* npc = npcs[i];
		npc->SetTarget(nullptr);
		npc->SetDefendAnchor(home);
		npc->SetRole(NpcNavigator::Role::DefendBase);
	}

	// 2) 残りを「穴に近い順」で攻撃割り当て
	std::vector<NPC*> rest(npcs.begin(),npcs.end());
	rest.erase(rest.begin(),std::min(rest.begin() + defenders,rest.end()));

	// 穴を取得
	const Boundary* boundary = Boundary::GetInstance();
	const auto& holes = boundary ? boundary->GetHoles() : std::vector<Hole>{};

	auto distToNearestHole = [&](NPC* u) -> float{
		const Vector3 p = u->GetWorldPosition();
		float best = std::numeric_limits<float>::infinity();
		for (const auto& h : holes){
			if (h.radius <= 0.0f) continue; // 無効穴はスキップ
			float d = (h.position - p).Length();
			if (d < best) best = d;
		}
		// 穴が無ければ敵拠点までの距離を代用
		if (!std::isfinite(best)) best = (rival - p).Length();
		return best;
	};

	std::sort(rest.begin(),rest.end(),
			  [&](NPC* a, NPC* b){ return distToNearestHole(a) < distToNearestHole(b); });

	const int attackersMax = std::min(attackers,(int)rest.size());
	for (int i = 0; i < attackersMax; ++i){
		auto* npc = rest[i];
		npc->ClearDefendAnchor();
		npc->SetTarget(pRivalStation_);
		npc->SetRole(NpcNavigator::Role::AttackBase);
	}
	for (int i = attackersMax; i < (int)rest.size(); ++i){
		auto* npc = rest[i];
		npc->SetTarget(nullptr);
		npc->SetDefendAnchor(home);
		npc->SetRole(NpcNavigator::Role::DefendBase);
	}
}

void BaseStation::CleanupSpawnedList(){
	auto it = std::remove_if(spawned_.begin(),spawned_.end(),
							 [](const NpcHandle& h){ return !h || !h->GetIsAlive(); });
	spawned_.erase(it,spawned_.end());
}

// ===== ここから：ルート走行（Outer固定 & 折り返し補間） =====
void BaseStation::MoveAlongRoute_(float dt) noexcept {
    if (!useRoute_ || !route_) return;

    const Spline* s = route_->GetSpline(routeActiveIndex_);
    if (!s) return;

    // ★ スプラインごとに弧長テーブルを用意
    if (!s->IsArcLengthBuilt()) { // ← なければ作るAPIを用意（なければ内部フラグでもOK）
        const_cast<Spline*>(s)->BuildArcLengthTable(400);
    }

    // ★ ルート全長とラップ時間から等速にする
    const float L = s->GetTotalLength();
    const float lap = (std::max)(0.1f, routeLapTime_);           // GUI 値
    const float speed = L / lap;                                // m/s（等速）
    traveledDist_ = std::fmod(traveledDist_ + speed * dt, L);   // 0..L

    // ★ 初期 phase を距離へ反映（初回のみ）
    if (!routeInitialized_) {
        routeInitialized_ = true;
        traveledDist_ = std::fmod(std::clamp(routePhase01_, 0.0f, 1.0f) * L, L);
        prevRoutePos_ = baseTransform_.translation_;
    }

    // 距離→t
    const float t = s->GetParamByDistance(traveledDist_);

    // 位置
    const Vector3 newPos = s->Evaluate(t);

    // 進行方向を向く
    Vector3 forward;
    bool gotTangent = false;

    // （接線 API があるならそちらを優先）
    // if (s->HasTangent()) { forward = s->Tangent(t).Normalize(); gotTangent = true; }

    if (!gotTangent) {
        Vector3 dir = newPos - prevRoutePos_;
        const float d = dir.Length();
        if (d > 1e-5f) { forward = dir / d; gotTangent = true; }
    }

    if (gotTangent) {
        auto YawPitchFromDir_LH = [](const Vector3& dir, float& yaw, float& pitch) {
            const float lenXZ = std::sqrt(dir.x*dir.x + dir.z*dir.z);
            yaw   = std::atan2(dir.x, dir.z);
            pitch = std::atan2(-dir.y, (lenXZ > 1e-6f ? lenXZ : 1e-6f));
        };
        float yaw = baseTransform_.rotation_.y;
        float pitch = baseTransform_.rotation_.x;
        YawPitchFromDir_LH(forward, yaw, pitch);
        baseTransform_.rotation_.y = yaw;
        baseTransform_.rotation_.x = pitch;
    }

    baseTransform_.translation_ = newPos;
    baseTransform_.UpdateMatrix();
    prevRoutePos_ = newPos;
}


Vector3 BaseStation::SampleVariantPos_(int variantIndex, float t01) const noexcept{
	if (!route_) return GetWorldPosition();
	if (variantIndex < 0){
		// 未設定時はRouteのactiveから
		return route_->Sample(t01);
	}
	const Spline* s = route_->GetSpline(variantIndex);
	if (!s) return route_->Sample(t01);
	// あなたの Spline 実装に合わせて（Sample / Evaluate / GetPoint）
	return s->Sample(t01);
}