#include "Actor/NPC/NPC.h"
#include "Actor/Boundary/Boundary.h"
#include "Actor/Station/Base/BaseStation.h"
#include "Actor/NPC/Bullet/Targeting.h"
#include "Actor/NPC/Bullet/FireController/NpcFierController.h"
#include "Frame/Frame.h"
#include "3d/ViewProjection.h"
#include "3d/Line3D.h"
#include "random.h"
#include "Navigation/RectXZWithGatesConstraint.h"
#include "imgui.h"
#include "Actor/Player/Bullet/BasePlayerBullet.h"
#include "Actor/NPC/Bullet/NpcBullet.h"

#include "Navigation/Route/RouteCollection.h"
#include "Navigation/Route/Route.h"

#include <limits>
#include <cmath>
#include <numbers>

namespace {

	inline float WrapPi(float a) {
		while (a > std::numbers::pi_v<float>) a -= 2.0f * std::numbers::pi_v<float>;
		while (a < -std::numbers::pi_v<float>) a += 2.0f * std::numbers::pi_v<float>;
		return a;
	}

	inline float MoveTowardsAngle(float current, float target, float maxStep) {
		float d = WrapPi(target - current);
		if (std::fabs(d) <= maxStep) return current + d;
		return current + std::copysign(maxStep, d);
	}

	// 安全正規化
	inline Vector3 SafeNormalize(const Vector3& v, const Vector3& fb = { 0,0,1 }) {
		float L = v.Length();
		return (L > 1e-6f) ? (v * (1.0f / L)) : fb;
	}


	// 回転(x=pitch, y=yaw, z=roll) から前方ベクトルを得る
	inline Vector3 ForwardFromPitchYaw(const Vector3& rot) {
		const float pitch = rot.x; // 上下
		const float yaw = rot.y; // 左右
		const float cp = std::cos(pitch), sp = std::sin(pitch);
		const float cy = std::cos(yaw), sy = std::sin(yaw);
		// 既存の yaw/pitch の使い方と整合する前方
		return SafeNormalize({ sy * cp, -sp, cy * cp });
	}

} // namespace

/// ===================================================
/// BoundaryHoleSource
/// ===================================================
const std::vector<Hole>& NPC::BoundaryHoleSource::GetHoles() const {
	static const std::vector<Hole> empty;
	return boundary ? boundary->GetHoles() : empty;
}

/// ===================================================
/// Ctor / Dtor
/// ===================================================
NPC::NPC() = default;
NPC::~NPC() = default;

/// ===================================================
/// Init
/// ===================================================
void NPC::Init() {
	globalParam_ = GlobalParameter::GetInstance();

	if (groupName_.empty()) { groupName_ = "UnnamedNPC"; }

	globalParam_->CreateGroup(groupName_, true);
	globalParam_->ClearBindingsForGroup(groupName_);
	BindParms();
	globalParam_->SyncParamForGroup(groupName_);
	// LoadData();

	Activate();

	// 航法初期化（自分の位置を中心に旋回開始）
	navigator_.Reset(GetWorldPosition());

	// npcの弾制御
	fireController_ = std::make_unique<NpcFireController>();
	fireController_->Init();

	// --- 通行制約（XZ 長方形＋穴ゲート）---
	Boundary* boundary = Boundary::GetInstance();
	holeSource_.boundary = boundary;
	moveConstraint_ = std::make_unique<RectXZWithGatesConstraint>(&holeSource_, boundary->GetRectXZWorld(), 0.01f);

	isInitialized_ = true;

	shootCooldown_ = Random::Range(0.0f, shootInterval_);

	BaseObject::Update();	//transformの更新を挟む

	/// hp
	hp_ = maxHP_;

#ifdef _DEBUG
	lineDrawer_ = std::make_unique<Line3D>();
	lineDrawer_->Init(256);
#endif // _DEBUG

	/// collision 
	cTransform_.Init();
	AABBCollider::SetCollisionScale(Vector3{ 1, 1, 1 } *10.0f);
}
/// ===================================================
/// UpdateS
/// ===================================================
void NPC::Update() {
	if (fireController_) fireController_->Tick();
	Move();
	TryFire();//座標などを更新してから
	BaseObject::Update();
	cTransform_.translation_ = GetWorldPosition();
	cTransform_.UpdateMatrix();
}

void NPC::DebugDraw([[maybe_unused]] const ViewProjection& vp) {
#ifdef _DEBUG
	if (!lineDrawer_) return;

	// ===== 視錐台の 8 頂点を計算 =====
	const Vector3 origin = GetWorldPosition();

	// 前方ベクトル（既存ヘルパー）
	const Vector3 f = ForwardFromPitchYaw(baseTransform_.rotation_);

	// up が f とほぼ平行なら代替Upを使う
	Vector3 upHint = { 0, 1, 0 };
	if (std::fabs(Vector3::Dot(f, upHint)) > 0.98f) upHint = { 0, 0, 1 };

	// 直交基底 right / up
	Vector3 r = Vector3::Cross(upHint, f);
	float rl = r.Length();
	r = (rl > 1e-6f) ? (r * (1.0f / rl)) : Vector3{ 1, 0, 0 };

	Vector3 u = Vector3::Cross(f, r);
	float ul = u.Length();
	u = (ul > 1e-6f) ? (u * (1.0f / ul)) : Vector3{ 0, 1, 0 };

	// FOV/距離
	const float radH = fireConeHFovDeg_ * 3.1415926535f / 180.0f;
	const float radV = fireConeVFovDeg_ * 3.1415926535f / 180.0f;

	const float nHalfW = fireConeNear_ * std::tan(radH);
	const float nHalfH = fireConeNear_ * std::tan(radV);
	const float fHalfW = fireConeFar_ * std::tan(radH);
	const float fHalfH = fireConeFar_ * std::tan(radV);

	const Vector3 Cn = origin + f * fireConeNear_;
	const Vector3 Cf = origin + f * fireConeFar_;

	// 近面4隅
	const Vector3 nTL = Cn + u * nHalfH - r * nHalfW;
	const Vector3 nTR = Cn + u * nHalfH + r * nHalfW;
	const Vector3 nBR = Cn - u * nHalfH + r * nHalfW;
	const Vector3 nBL = Cn - u * nHalfH - r * nHalfW;

	// 遠面4隅
	const Vector3 fTL = Cf + u * fHalfH - r * fHalfW;
	const Vector3 fTR = Cf + u * fHalfH + r * fHalfW;
	const Vector3 fBR = Cf - u * fHalfH + r * fHalfW;
	const Vector3 fBL = Cf - u * fHalfH - r * fHalfW;

	// ===== あなたの流儀：Reset → SetLine 群 → Draw =====
	const Vector4 col = { 1.0f, 0.6f, 0.1f, 1.0f }; // 視錐台の色（お好みで）

	lineDrawer_->Reset();

	// 近面(4)
	lineDrawer_->SetLine(nTL, nTR, col);
	lineDrawer_->SetLine(nTR, nBR, col);
	lineDrawer_->SetLine(nBR, nBL, col);
	lineDrawer_->SetLine(nBL, nTL, col);

	// 遠面(4)
	lineDrawer_->SetLine(fTL, fTR, col);
	lineDrawer_->SetLine(fTR, fBR, col);
	lineDrawer_->SetLine(fBR, fBL, col);
	lineDrawer_->SetLine(fBL, fTL, col);

	// 側面(4)
	lineDrawer_->SetLine(nTL, fTL, col);
	lineDrawer_->SetLine(nTR, fTR, col);
	lineDrawer_->SetLine(nBR, fBR, col);
	lineDrawer_->SetLine(nBL, fBL, col);

	// 補助線が欲しければ（任意）
	lineDrawer_->SetLine(origin, nTL, col);
	lineDrawer_->SetLine(origin, nTR, col);
	lineDrawer_->SetLine(origin, nBR, col);
	lineDrawer_->SetLine(origin, nBL, col);

	lineDrawer_->Draw(vp);
#endif

}

/// ===================================================
/// Accessors / Controls
/// ===================================================
void NPC::SetTarget(const BaseStation* target) {
	target_ = target;
	// 攻撃指示を受けたら防衛アンカーは解除
	if (target_) { hasDefendAnchor_ = false; defendAnchor_ = {}; }
}

NpcFireController* NPC::GetFireController() const{return fireController_.get();}
void NPC::Activate() { isActive_ = true; }
void NPC::Deactivate() { isActive_ = false; }

void NPC::SetDefendAnchor(const Vector3& p) {
	defendAnchor_ = p;
	hasDefendAnchor_ = true;
}

void NPC::ClearDefendAnchor() {
	hasDefendAnchor_ = false;
	defendAnchor_ = {};
}

/// ===================================================
/// Movement / Navigation
/// ===================================================
void NPC::StartOrbit(const Vector3& center) {
	navigator_.StartOrbit(center);
}
void NPC::Move() {
	if (!isActive_) return;

	const float dt = Frame::DeltaTime();
	const Vector3 npcPos = GetWorldPosition();

	NpcNavigator::StationSide side{};
	side.allyBase = hasDefendAnchor_ ? defendAnchor_ : npcPos;   // 防衛アンカー or 現在地
	side.enemyBase = target_ ? target_->GetWorldPosition() : npcPos;
	navigator_.SetStationSide(side);

	Vector3 sensedTgt = npcPos;
	if (const BaseObject* ft = PickFrustumTarget()) {
		sensedTgt = ft->GetWorldPosition();
	} else if (target_) {
		sensedTgt = side.enemyBase;
	}

	const Boundary* boundary = Boundary::GetInstance();
	const std::vector<Hole>& holes = boundary->GetHoles();

	if (speed_ != navConfig_.speed) { navConfig_.speed = speed_; }

	// ★追加：状態遷移の検知
	auto prevState = navigator_.GetState();

	// 役割駆動ナビゲーション
	const Vector3 desiredDelta = navigator_.Tick(dt, npcPos, sensedTgt, holes);

	// ★Orbit へ入った瞬間にスプラインを中心へ合わせてバインド
	if (prevState != NpcNavigator::State::Orbit &&
		navigator_.GetState() == NpcNavigator::State::Orbit) {
		const Vector3 center = hasDefendAnchor_ ? defendAnchor_ : npcPos;
		BindOrbitRouteAtEntry_(center);
	}

	// ---- 目的地へ移動（制約を通す）----
	Vector3 from = baseTransform_.translation_;
	Vector3 to = from + desiredDelta;

	if (moveConstraint_) { to = moveConstraint_->FilterMove(from, to); }

	// === 進行方向へ機体を向ける（元の実装のまま） ===
	const Vector3 v = to - from;
	const float vLen = v.Length();
	if (vLen > 1e-6f) {
		const Vector3 dir = v * (1.0f / vLen);

		const float targetYaw = std::atan2(dir.x, dir.z);
		const float targetPitch = std::atan2(-dir.y, std::sqrt(dir.x * dir.x + dir.z * dir.z));

		const float turnRateRadPerSec = std::numbers::pi_v<float> *2.0f;
		const float maxStep = turnRateRadPerSec * dt;

		baseTransform_.rotateOder_ = RotateOder::XYZ;

		Vector3& rot = baseTransform_.rotation_;
		rot.y = MoveTowardsAngle(rot.y, targetYaw, maxStep);
		rot.x = MoveTowardsAngle(rot.x, targetPitch, maxStep);

		const float bankGain = 0.6f;
		const float bankMax = std::numbers::pi_v<float> *0.35f;
		float yawErr = WrapPi(targetYaw - rot.y);
		float targetBank = std::clamp(-yawErr * bankGain, -bankMax, bankMax);
		const float bankRate = std::numbers::pi_v<float> *1.2f;
		rot.z = MoveTowardsAngle(rot.z, targetBank, bankRate * dt);
	}

	baseTransform_.translation_ = to;
}

// Orbit突入時にスプラインを中心へ平行移動してバインド
void NPC::BindOrbitRouteAtEntry_(const Vector3& center) {
	if (!routes_) return;

	// 陣営×ロール → RouteType
	RouteType rt;
	if (faction_ == FactionType::Ally) {
		rt = (role_ == NpcNavigator::Role::DefendBase) ? RouteType::AllyDifence : RouteType::AllyAttack;
	} else {
		rt = (role_ == NpcNavigator::Role::DefendBase) ? RouteType::EnemyDirence : RouteType::EnemyAttack;
	}

	// routes_ は const で保持しているので使用時だけ外す（最小差分）
	auto* rc = const_cast<RouteCollection*>(routes_);
	if (auto* route = rc->GetRoute(rt)) {
		route->SetBaseOffset(center);
		navigator_.BindOrbitRoute(route);
		navigator_.SelectInitialOrbitRoute();
	}
}

/// ===================================================
/// Fire
/// ===================================================
void NPC::TryFire() {
	shootCooldown_ -= Frame::DeltaTime();
	if (shootCooldown_ > 0.0f) return;
	if (!targetProvider_) return;
	if (!fireController_) return;

	// 射撃候補を集める
	std::vector<const BaseObject*> candidates;
	targetProvider_->CollectTargets(candidates);
	if (candidates.empty()) return;

	// 最適ターゲットを選ぶ
	const BaseObject* target = PickFrustumTarget();
	if (!target) return;

	const Vector3 muzzle = GetWorldPosition();

	switch (fireMode_) {
		case FireMode::Homing:
			{
				// Homing: 目標を追尾
				Vector3 dir = (target->GetWorldPosition() - muzzle);
				fireController_->SpawnHoming(muzzle, dir, target);
			}
			break;
		case FireMode::Straight:
		default:
			{
			// 自機の回転から前方ベクトルを得る
			Vector3 forward = ForwardFromPitchYaw(baseTransform_.rotation_);
			if (forward.Length() < 1e-6f){
				forward = Vector3(0, 0, 1); // 万一ゼロならデフォルト前方
			}
			fireController_->SpawnStraight(muzzle, forward.Normalize());
			}
			break;
	}

	shootCooldown_ = shootInterval_;
}

bool NPC::IsInFiringFrustum(const Vector3& worldPt) const{
	const Vector3 origin = GetWorldPosition();
	const Vector3 v = worldPt - origin;

	// 距離（near/far）
	const float d2 = Vector3::Dot(v, v);
	if (d2 < fireConeNear_ * fireConeNear_) return false;
	if (d2 > fireConeFar_ * fireConeFar_) return false;

	// 前方ベクトル
	const Vector3 f = ForwardFromPitchYaw(baseTransform_.rotation_);

	// f とほぼ平行な up を避ける
	Vector3 upHint = {0, 1, 0};
	if (std::fabs(Vector3::Dot(f, upHint)) > 0.98f) upHint = {0, 0, 1};

	// 直交基底 right / up
	Vector3 r = Vector3::Cross(upHint, f);
	const float rl = r.Length();
	r = (rl > 1e-6f) ? (r * (1.0f / rl)) : Vector3 {1,0,0};

	Vector3 u = Vector3::Cross(f, r);
	const float ul = u.Length();
	u = (ul > 1e-6f) ? (u * (1.0f / ul)) : Vector3 {0,1,0};

	// v を基底に投影
	const float zf = Vector3::Dot(v, f); // 前後成分（前方が正）
	if (zf <= 0.0f) return false;        // 背面は不可

	const float xr = Vector3::Dot(v, r); // 右左成分
	const float yu = Vector3::Dot(v, u); // 上下成分

	const float tanH = std::tan(fireConeHFovDeg_ * 3.1415926535f / 180.0f);
	const float tanV = std::tan(fireConeVFovDeg_ * 3.1415926535f / 180.0f);

	if (std::fabs(xr) > zf * tanH) return false; // 水平方向に外
	if (std::fabs(yu) > zf * tanV) return false; // 垂直方向に外

	return true;
}

const BaseObject* NPC::PickFrustumTarget() const{
	if (!targetProvider_) return nullptr;

	std::vector<const BaseObject*> candidates;
	targetProvider_->CollectTargets(candidates);
	if (candidates.empty()) return nullptr;

	const Vector3 p = GetWorldPosition();
	const BaseObject* best = nullptr;
	float bestD2 = std::numeric_limits<float>::infinity();

	for (auto* c : candidates){
		if (!c) continue;
		const Vector3 cp = c->GetWorldPosition();
		if (!IsInFiringFrustum(cp)) continue;

		const float d2 = Vector3::Dot(cp - p, cp - p);
		if (d2 < bestD2){ bestD2 = d2; best = c; }
	}
	return best;
}

/// ===================================================
/// Collision
/// ===================================================
void NPC::OnCollisionEnter(BaseCollider* other) {

	/// 敵のNPCにプレイヤー弾が当たったらダメージ
	if (faction_ == FactionType::Enemy) {
		if (BasePlayerBullet* bullet = dynamic_cast<BasePlayerBullet*>(other)) {
			float damage = bullet->GetParameter().damage;
			hp_ -= damage;
			if (hp_ <= 0.0f) {
				hp_ = 0.0f;
				// 死亡処理
				Deactivate();
				/// エフェクトの生成
			}
		}
		return;
	}

	// 味方NPCに敵弾が当たったらダメージ
	if (faction_ == FactionType::Ally) {
		/// 敵弾のクラスをここに追加
		if (NpcBullet* bullet = dynamic_cast<NpcBullet*>(other)) {
			float damage = bullet->GetDamage();
			hp_ -= damage;
			if (hp_ <= 0.0f) {
				hp_ = 0.0f;
				// 死亡処理
				Deactivate();
			}
		}
		return;
	}

}

void NPC::AttachRoutes(const RouteCollection* rc) noexcept {
	routes_ = rc;
	// 既にOrbit中なら即バインドしてスプライン移動を始める
	if (navigator_.GetState() == NpcNavigator::State::Orbit) {
		const Vector3 center = hasDefendAnchor_ ? defendAnchor_ : GetWorldPosition();
		BindOrbitRouteAtEntry_(center);
	}
}

/// ===================================================
/// Param I/O
/// ===================================================
void NPC::BindParms() {
	 globalParam_->Bind(groupName_, "shootInterval", &shootInterval_);
	 globalParam_->Bind(groupName_, "fireConeFar", &fireConeFar_);
}

void NPC::LoadData() {
	const std::string path = fileDirectory_;
	globalParam_->LoadFile(groupName_, path);
	globalParam_->SyncParamForGroup(groupName_);
}

void NPC::SaveData() {
	const std::string path = fileDirectory_;
	globalParam_->SaveFile(groupName_, path);
}