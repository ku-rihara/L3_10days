#include "NPC.h"

#include "Actor/Station/Base/BaseStation.h"
#include "Actor/Boundary/Boundary.h"
#include "Actor/NPC/Bullet/FireController/NpcFierController.h"
#include "Navigation/RectXZWithGatesConstraint.h"
#include "Frame/Frame.h"
#include "random.h"

#include <cmath>
#include <numbers>
#include <algorithm>
#include <limits>

namespace {

inline float WrapPi(float a){
	while (a > std::numbers::pi_v<float>) a -= 2.0f * std::numbers::pi_v<float>;
	while (a < -std::numbers::pi_v<float>) a += 2.0f * std::numbers::pi_v<float>;
	return a;
}

inline float MoveTowardsAngle(float current, float target, float maxStep){
	float d = WrapPi(target - current);
	if (std::fabs(d) <= maxStep) return current + d;
	return current + std::copysign(maxStep,d);
}

// 安全正規化
inline Vector3 SafeNormalize(const Vector3& v, const Vector3& fb = {0,0,1}) {
	float L = v.Length();
	return (L > 1e-6f) ? (v * (1.0f/L)) : fb;
}

// 内積・外積（Vector3 に静的関数がない前提で用意）
inline float DotV(const Vector3& a, const Vector3& b){ return a.x*b.x + a.y*b.y + a.z*b.z; }
inline Vector3 CrossV(const Vector3& a, const Vector3& b){
	return { a.y*b.z - a.z*b.y, a.z*b.x - a.x*b.z, a.x*b.y - a.y*b.x };
}

// 回転(x=pitch, y=yaw, z=roll) から前方ベクトルを得る
inline Vector3 ForwardFromPitchYaw_(const Vector3& rot){
	const float pitch = rot.x; // 上下
	const float yaw   = rot.y; // 左右
	const float cp = std::cos(pitch), sp = std::sin(pitch);
	const float cy = std::cos(yaw),   sy = std::sin(yaw);
	// 既存の yaw/pitch の使い方と整合する前方
	return SafeNormalize({ sy * cp, -sp, cy * cp });
}

} // namespace

// ===== BoundaryHoleSource 実装 =====
const std::vector<Hole>& NPC::BoundaryHoleSource::GetHoles() const{
	static const std::vector<Hole> kEmpty;
	return boundary ? boundary->GetHoles() : kEmpty;
}

NPC::~NPC() = default;

/// ===================================================
/// 初期化
/// ===================================================
void NPC::Init(){
	globalParam_ = GlobalParameter::GetInstance();

	if (groupName_.empty()){ groupName_ = "UnnamedNPC"; }

	globalParam_->CreateGroup(groupName_,true);
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
	RectXZ rect{-1500.0f, 1500.0f, -1500.0f, 1500.0f};
	moveConstraint_ = std::make_unique<RectXZWithGatesConstraint>(&holeSource_,rect,0.01f);

	isInitialized_ = true;

	shootCooldown_ = Random::Range(0.0f,shootInterval_);
}

/// ===================================================
/// 更新
/// ===================================================
void NPC::Update(){
	if (fireController_) fireController_->Tick();
	Move();
	TryFire();
	BaseObject::Update();
}

/// ===================================================
/// 移動（Navigator 出力 → Constraint で検閲 → 反映）
/// ===================================================
void NPC::Move(){
	if (!isActive_) return;

	const float dt = Frame::DeltaTime();

	const Vector3 npcPos = GetWorldPosition();
	const Vector3 tgtPos = (target_)
		                       ? target_->GetWorldPosition()
		                       : (hasDefendAnchor_ ? defendAnchor_ : npcPos);

	const Boundary* boundary = Boundary::GetInstance();
	const std::vector<Hole>& holes = boundary->GetHoles();

	if (speed_ != navConfig_.speed){ navConfig_.speed = speed_; }

	const Vector3 desiredDelta = navigator_.Tick(dt,npcPos,tgtPos,holes);

	Vector3 from = baseTransform_.translation_;
	Vector3 to = from + desiredDelta;

	if (moveConstraint_){ to = moveConstraint_->FilterMove(from,to); }

	// === 進行方向へ向ける ===
	const Vector3 v = to - from;
	const float vLen = v.Length();
	if (vLen > 1e-6f){
		const Vector3 dir = v * (1.0f / vLen);

		const float targetYaw = std::atan2(dir.x,dir.z);
		const float targetPitch = std::atan2(-dir.y,std::sqrt(dir.x * dir.x + dir.z * dir.z));

		const float turnRateRadPerSec = std::numbers::pi_v<float> * 2.0f;
		const float maxStep = turnRateRadPerSec * dt;

		baseTransform_.rotateOder_ = RotateOder::XYZ;

		Vector3& rot = baseTransform_.rotation_; // (x=pitch, y=yaw, z=roll)
		rot.y = MoveTowardsAngle(rot.y,targetYaw,maxStep);
		rot.x = MoveTowardsAngle(rot.x,targetPitch,maxStep);

		const float bankGain = 0.6f;
		const float bankMax = std::numbers::pi_v<float> * 0.35f;
		float yawErr = WrapPi(targetYaw - rot.y);
		float targetBank = std::clamp(-yawErr * bankGain,-bankMax,bankMax);
		const float bankRate = std::numbers::pi_v<float> * 1.2f;
		rot.z = MoveTowardsAngle(rot.z,targetBank,bankRate * dt);
	}

	// 位置反映
	baseTransform_.translation_ = to;

	// 防衛待機（ターゲット無し）時は、アンカーがあればアンカー中心に旋回
	if (!target_){
		const Vector3 center = hasDefendAnchor_ ? defendAnchor_ : npcPos;
		StartOrbit(center);
	}
}

/// ===================================================
/// 視錐台内チェック
/// ===================================================
bool NPC::IsInFiringFrustum_(const Vector3& worldPt) const {
	// 基底座標
	const Vector3 npcPos = GetWorldPosition();
	const Vector3 fwd = ForwardFromPitchYaw_(baseTransform_.rotation_);

	// 右・上ベクトル（ワールドUpを使って直交基底を作る）
	Vector3 worldUp = {0,1,0};
	if (std::fabs(DotV(fwd, worldUp)) > 0.98f) worldUp = {0,0,1}; // 平行回避

	const Vector3 right = SafeNormalize(CrossV(worldUp, fwd));
	const Vector3 up    = SafeNormalize(CrossV(fwd, right));

	// ターゲットをNPCローカルへ投影
	const Vector3 to = worldPt - npcPos;
	const float x = DotV(to, right);
	const float y = DotV(to, up);
	const float z = DotV(to, fwd);   // 前方正

	// Z（距離）チェック
	if (z < fireConeNear_ || z > fireConeFar_) return false;

	// 視錐台境界：|x| <= z * tan(hHFov), |y| <= z * tan(vHFov)
	const float tanHx = std::tan(fireConeHFovDeg_ * 3.14159265f / 180.0f);
	const float tanHy = std::tan(fireConeVFovDeg_ * 3.14159265f / 180.0f);
	if (std::fabs(x) > z * tanHx) return false;
	if (std::fabs(y) > z * tanHy) return false;

	return true;
}

/// ===================================================
/// 視錐台から最適ターゲットを選ぶ
/// ===================================================
const BaseObject* NPC::PickFrustumTarget_() const {
	if (!targetProvider_) return nullptr;

	// 候補収集
	std::vector<const BaseObject*> candidates;
	candidates.reserve(32);
	targetProvider_->CollectTargets(candidates);
	if (candidates.empty()) return nullptr;

	// 射出座標系基底
	const Vector3 npcPos = GetWorldPosition();
	const Vector3 fwd = ForwardFromPitchYaw_(baseTransform_.rotation_);
	Vector3 worldUp = {0,1,0};
	if (std::fabs(DotV(fwd, worldUp)) > 0.98f) worldUp = {0,0,1};
	const Vector3 right = SafeNormalize(CrossV(worldUp, fwd));
	const Vector3 up    = SafeNormalize(CrossV(fwd, right));

	const float tanHx = std::tan(fireConeHFovDeg_ * 3.14159265f / 180.0f);
	const float tanHy = std::tan(fireConeVFovDeg_ * 3.14159265f / 180.0f);

	const BaseObject* best = nullptr;
	float bestScore = std::numeric_limits<float>::infinity();

	for (auto* obj : candidates){
		if (!obj || obj == this) continue;

		// 視錐台内か？
		const Vector3 to = obj->GetWorldPosition() - npcPos;
		const float x = DotV(to, right);
		const float y = DotV(to, up);
		const float z = DotV(to, fwd);
		if (z < fireConeNear_ || z > fireConeFar_) continue;
		if (std::fabs(x) > z * tanHx) continue;
		if (std::fabs(y) > z * tanHy) continue;

		// 中心に近いほどスコア小（正面優先）、同率なら近距離優先
		const float nx = (tanHx > 1e-6f) ? (x / (z * tanHx)) : 0.0f; // [-1,1] 付近
		const float ny = (tanHy > 1e-6f) ? (y / (z * tanHy)) : 0.0f;
		const float centerCost = std::fabs(nx) + std::fabs(ny);
		const float distCost   = 0.001f * z;       // わずかに距離も加味
		const float score = centerCost + distCost;

		if (score < bestScore){
			bestScore = score;
			best = obj;
		}
	}

	return best;
}

/// ===================================================
/// 発砲（視錐台内の相手をターゲットに設定して撃つ）
/// ===================================================
void NPC::TryFire(){
	if (!fireController_) return;

	shootCooldown_ -= Frame::DeltaTime();
	if (shootCooldown_ > 0.0f) return;

	// 視錐台からターゲットを選ぶ
	const BaseObject* chosen = PickFrustumTarget_();
	if (!chosen) return;

	// 発射位置と向き：NPC の現在向き
	const Vector3 pos = GetWorldPosition();
	const Vector3 forward = ForwardFromPitchYaw_(baseTransform_.rotation_);

	// モードに応じて撃ち分け
	switch (fireMode_) {
	case FireMode::Homing:
		// ホーミング：視錐台で選んだ相手をターゲットに設定
		fireController_->SpawnHoming(pos, forward, chosen);
		break;
	case FireMode::Straight:
	default:
		// 直進：向いている方向へ
		fireController_->SpawnStraight(pos, forward);
		break;
	}

	shootCooldown_ = shootInterval_;
}

/// ===================================================
/// 旋回開始（Navigator へ委譲）
/// ===================================================
void NPC::StartOrbit(const Vector3& center){ navigator_.StartOrbit(center); }

/////////////////////////////////////////////////////////////////////////////////////////
//      パラメータ
/////////////////////////////////////////////////////////////////////////////////////////
void NPC::BindParms(){
	globalParam_->Bind(groupName_,"maxHP",&maxHP_);
	globalParam_->Bind(groupName_,"speed",&speed_);
	globalParam_->Bind(groupName_,"shootInterval",&shootInterval_);

	// 視錐台パラメータ
	globalParam_->Bind(groupName_,"fireConeNear",&fireConeNear_);
	globalParam_->Bind(groupName_,"fireConeFar",&fireConeFar_);
	globalParam_->Bind(groupName_,"fireConeHFovDeg",&fireConeHFovDeg_);
	globalParam_->Bind(groupName_,"fireConeVFovDeg",&fireConeVFovDeg_);
	globalParam_->Bind(groupName_,"fireConeDebug",&fireConeDebug_);
}

void NPC::LoadData(){ globalParam_->LoadFile(groupName_,fileDirectory_); }
void NPC::SaveData(){ globalParam_->SaveFile(groupName_,fileDirectory_); }

/////////////////////////////////////////////////////////////////////////////////////////
//      accessor
/////////////////////////////////////////////////////////////////////////////////////////
void NPC::SetTarget(const BaseStation* target){ target_ = target; }
void NPC::Activate(){ isActive_ = true; }
void NPC::Deactivate(){ isActive_ = false; }

void NPC::SetDefendAnchor(const Vector3& p){
	defendAnchor_ = p;
	hasDefendAnchor_ = true;
}

void NPC::ClearDefendAnchor(){ hasDefendAnchor_ = false; }