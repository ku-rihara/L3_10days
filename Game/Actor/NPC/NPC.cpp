#include "NPC.h"

#include "Actor/Station/Base/BaseStation.h"
#include "Actor/Boundary/Boundary.h"
#include "Frame/Frame.h"

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
}

// ===== BoundaryHoleSource 実装 =====
const std::vector<Hole>& NPC::BoundaryHoleSource::GetHoles() const {
	static const std::vector<Hole> kEmpty;
	return boundary ? boundary->GetHoles() : kEmpty;
}

/// ===================================================
/// 初期化
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

	// --- 通行制約（XZ 長方形＋穴ゲート）---
	Boundary* boundary = Boundary::GetInstance();
	holeSource_.boundary = boundary;
	RectXZ rect{ -1500.0f, 1500.0f, -1500.0f, 1500.0f };
	moveConstraint_ = std::make_unique<RectXZWithGatesConstraint>(&holeSource_, rect, 0.01f);

	isInitialized_ = true;
}

/// ===================================================
/// 更新
/// ===================================================
void NPC::Update() {
	Move();
	BaseObject::Update();
}

/// ===================================================
/// 移動（Navigator 出力 → Constraint で検閲 → 反映）
/// ===================================================
void NPC::Move() {
	if (!isActive_) return;

	const float dt = Frame::DeltaTime();

	const Vector3 npcPos = GetWorldPosition();
	// ★ target が無いとき、アンカーがあればアンカーを目標に扱う
	const Vector3 tgtPos = (target_)
		? target_->GetWorldPosition()
		: (hasDefendAnchor_ ? defendAnchor_ : npcPos);

	const Boundary* boundary = Boundary::GetInstance();
	const std::vector<Hole>& holes = boundary->GetHoles();

	if (speed_ != navConfig_.speed) {
		navConfig_.speed = speed_;
	}

	const Vector3 desiredDelta = navigator_.Tick(dt, npcPos, tgtPos, holes);

	Vector3 from = baseTransform_.translation_;
	Vector3 to = from + desiredDelta;

	if (moveConstraint_) {
		to = moveConstraint_->FilterMove(from, to);
	}

	// === 進行方向へ向ける ===
	const Vector3 v = to - from;
	const float   vLen = v.Length();
	if (vLen > 1e-6f) {
		const Vector3 dir = v / vLen;

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

	// ★ 防衛待機（ターゲット無し）時は、アンカーがあればアンカー中心に旋回
	if (!target_) {
		const Vector3 center = hasDefendAnchor_ ? defendAnchor_ : npcPos;
		StartOrbit(center);
	}
}

/// ===================================================
/// 旋回開始（Navigator へ委譲）
/// ===================================================
void NPC::StartOrbit(const Vector3& center) { navigator_.StartOrbit(center); }

/////////////////////////////////////////////////////////////////////////////////////////
//      パラメータ
/////////////////////////////////////////////////////////////////////////////////////////

void NPC::BindParms() {
	globalParam_->Bind(groupName_, "maxHP", &maxHP_);
	globalParam_->Bind(groupName_, "speed", &speed_);
}

void NPC::LoadData() { globalParam_->LoadFile(groupName_, fileDirectory_); }
void NPC::SaveData() { globalParam_->SaveFile(groupName_, fileDirectory_); }

/////////////////////////////////////////////////////////////////////////////////////////
//      accessor
/////////////////////////////////////////////////////////////////////////////////////////

void NPC::SetTarget(const BaseStation* target) { target_ = target; }
void NPC::SetFaction(FactionType faction) { faction_ = faction; }
void NPC::Activate() { isActive_ = true; }
void NPC::Deactivate() { isActive_ = false; }

// ★ 追加：防衛アンカー API
void NPC::SetDefendAnchor(const Vector3& p) {
	defendAnchor_ = p;
	hasDefendAnchor_ = true;
}
void NPC::ClearDefendAnchor() {
	hasDefendAnchor_ = false;
}
