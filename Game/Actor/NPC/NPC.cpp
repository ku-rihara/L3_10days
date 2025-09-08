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
	RectXZ rect{ -1500.0f, 1500.0f, -1500.0f, 1500.0f };
	moveConstraint_ = std::make_unique<RectXZWithGatesConstraint>(&holeSource_, rect, 0.01f);

	isInitialized_ = true;

	shootCooldown_ = Random::Range(0.0f, shootInterval_);

	BaseObject::Update();	//transformの更新を挟む

#ifdef _DEBUG
	lineDrawer_ = std::make_unique<Line3D>();
	lineDrawer_->Init(256);
#endif // _DEBUG

	/// collision 
	cTransform_.Init();
	AABBCollider::SetCollisionScale(Vector3{ 1, 1, 1 } *100.0f);
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
	const Vector3 tgtPos = (target_)
		? target_->GetWorldPosition()
		: (hasDefendAnchor_ ? defendAnchor_ : npcPos);

	const Boundary* boundary = Boundary::GetInstance();
	const std::vector<Hole>& holes = boundary->GetHoles();

	if (speed_ != navConfig_.speed) { navConfig_.speed = speed_; }

	const Vector3 desiredDelta = navigator_.Tick(dt, npcPos, tgtPos, holes);

	Vector3 from = baseTransform_.translation_;
	Vector3 to = from + desiredDelta;

	if (moveConstraint_) { to = moveConstraint_->FilterMove(from, to); }

	// === 進行方向へ向ける ===
	const Vector3 v = to - from;
	const float vLen = v.Length();
	if (vLen > 1e-6f) {
		const Vector3 dir = v * (1.0f / vLen);

		const float targetYaw = std::atan2(dir.x, dir.z);
		const float targetPitch = std::atan2(-dir.y, std::sqrt(dir.x * dir.x + dir.z * dir.z));

		const float turnRateRadPerSec = std::numbers::pi_v<float> *2.0f;
		const float maxStep = turnRateRadPerSec * dt;

		baseTransform_.rotateOder_ = RotateOder::XYZ;

		Vector3& rot = baseTransform_.rotation_; // (x=pitch, y=yaw, z=roll)
		rot.y = MoveTowardsAngle(rot.y, targetYaw, maxStep);
		rot.x = MoveTowardsAngle(rot.x, targetPitch, maxStep);

		const float bankGain = 0.6f;
		const float bankMax = std::numbers::pi_v<float> *0.35f;
		float yawErr = WrapPi(targetYaw - rot.y);
		float targetBank = std::clamp(-yawErr * bankGain, -bankMax, bankMax);
		const float bankRate = std::numbers::pi_v<float> *1.2f;
		rot.z = MoveTowardsAngle(rot.z, targetBank, bankRate * dt);
	}

	// 位置反映
	baseTransform_.translation_ = to;

	// 防衛待機（ターゲット無し）時は、アンカーがあればアンカー中心に旋回
	if (!target_) {
		const Vector3 center = hasDefendAnchor_ ? defendAnchor_ : npcPos;
		StartOrbit(center);
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

	// 射撃候補を集める（敵NPC + 敵拠点）
	std::vector<const BaseObject*> candidates;
	targetProvider_->CollectTargets(candidates);
	if (candidates.empty()) return;

	// 最適ターゲットを選ぶ（とりあえず「最も近い & 射程内」）
	const BaseObject* target = PickFrustumTarget();
	if (!target) return;

	const Vector3 muzzle = GetWorldPosition();

	// 実弾発射：既存の NpcFierController API に合わせて呼び出し
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
			// Straight: 前方へ直進弾
			// 前方ベクトルが未確定なら目標方向で打つ
			{
				Vector3 dir = (target->GetWorldPosition() - muzzle);
				if (dir.LengthSq() < 1e-6f) dir = Vector3(0, 0, 1);
				dir.Normalize();
				fireController_->SpawnStraight(muzzle, dir);
			}
			break;
	}

	shootCooldown_ = shootInterval_;
}

/// 視錐台チェック（前方ベクトルの取り方が未共有のため、まずは距離のみ判定）
bool NPC::IsInFiringFrustum(const Vector3& worldPt) const {
	const Vector3 p = GetWorldPosition();
	const float d2 = (worldPt - p).LengthSq();
	if (d2 < fireConeNear_ * fireConeNear_) return false;
	if (d2 > fireConeFar_ * fireConeFar_)  return false;

	// TODO: 前方ベクトル（例：baseTransform_ から forward を取得）と角度で
	// HFOV/VFOV を掛けたい場合はここで判定を拡張
	return true;
}

/// 最適ターゲット（最も近い射程内）
/// targetProvider_ から都度取得する実装に合わせ、ここで再取得する
const BaseObject* NPC::PickFrustumTarget() const {
	if (!targetProvider_) return nullptr;

	std::vector<const BaseObject*> candidates;
	targetProvider_->CollectTargets(candidates);
	if (candidates.empty()) return nullptr;

	const Vector3 p = GetWorldPosition();
	const BaseObject* best = nullptr;
	float bestD2 = std::numeric_limits<float>::infinity();

	for (auto* c : candidates) {
		if (!c) continue;
		const Vector3 cp = c->GetWorldPosition();
		if (!IsInFiringFrustum(cp)) continue;

		const float d2 = (cp - p).LengthSq();
		if (d2 < bestD2) { bestD2 = d2; best = c; }
	}
	return best;
}

/// ===================================================
/// Collision
/// ===================================================
void NPC::OnCollisionEnter(BaseCollider* other) {
	// TODO: 弾との衝突で hp を減らす等
	(void)other;

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
