#include "NpcHomingBullet.h"
#include "utility/ParameterEditor/GlobalParameter.h"
#include "Frame/Frame.h"

static inline float Clamp01(float v) { return (v < 0.f) ? 0.f : (v > 1.f ? 1.f : v); }

// 
static Vector3 RotateAroundAxis(const Vector3& v, const Vector3& axisN, float angleRad) {
	float c = std::cos(angleRad), s = std::sin(angleRad);
	return v * c + Vector3::Cross(axisN, v) * s + axisN * (Vector3::Dot(axisN, v) * (1.0f - c));
}

// 回す
static Vector3 RotateTowards(const Vector3& dir, const Vector3& desired, float maxRad) {
	Vector3 d0 = Vector3::Normalize(dir);
	Vector3 d1 = (desired, d0).Normalize();
	float dot = Clamp01(Vector3::Dot(d0, d1));
	dot = std::min(1.0f, (std::max)(-1.0f, dot));
	float ang = std::acos(dot);
	if (ang < 1e-4f) return d1; // ほぼ一致

	float step = std::min(maxRad, ang);
	Vector3 axis = Vector3::Cross(d0, d1);
	float axisLen = axis.Length();
	if (axisLen < 1e-6f) {
		axis = std::fabs(d0.y) < 0.99f ? Vector3::Cross(d0, { 0,1,0 }) : Vector3::Cross(d0, { 1,0,0 });
		axis = Vector3::Normalize(axis);
	} else {
		axis = axis * (1.0f / axisLen);
	}
	return Vector3::Normalize(RotateAroundAxis(d0, axis, step));
}


void NpcHomingBullet::Init() {
	// 名前の初期化
	groupName_ = "NpcHomingBullet";
	modelName_ = "cube.obj";

	// パラメータとモデルの初期化
	NpcBullet::Init();

	BindParms();
	if (globalParam_) globalParam_->SyncParamForGroup(groupName_);
}

void NpcHomingBullet::BindParms() {
	NpcBullet::BindParms();

	if (!globalParam_) return;
	globalParam_->Bind(groupName_, "turnRateDegPerSec", &turnRateDegPerSec_);
	globalParam_->Bind(groupName_, "loseAngleDeg", &loseAngleDeg_);
	globalParam_->Bind(groupName_, "loseDistance", &loseDistance_);
	globalParam_->Bind(groupName_, "keepLockInsideCone", &keepLockInsideCone_);
}

void NpcHomingBullet::SteerToTarget(float dt) {
	if (!target_) return;

	const Vector3 myPos = baseTransform_.translation_;
	const Vector3 toTgt = target_->GetWorldPosition() - myPos;
	const float   dist = toTgt.Length();

	// 距離条件：遠すぎたらロック解除
	if (dist > loseDistance_) { target_ = nullptr; return; }

	const Vector3 desired = Vector3::Normalize(toTgt);
	float cosAng = Vector3::Dot(dir_, desired);
	cosAng = std::min(1.0f, (std::max)(-1.0f, cosAng));
	float angDeg = std::acos(cosAng) * 180.0f / 3.14159265f;

	if (angDeg > loseAngleDeg_ && keepLockInsideCone_) { target_ = nullptr; return; }

	// 方向を「最大角速度」内で目標へ寄せる
	const float maxTurnRad = (turnRateDegPerSec_ * dt) * 3.14159265f / 180.0f;
	dir_ = RotateTowards(dir_, desired, maxTurnRad);
}

void NpcHomingBullet::Update() {
	if (!isActive_) return;

	const float dt = Frame::DeltaTime();

	//ターゲットがいなければそのまま直進
	if (target_) {
		SteerToTarget(dt);
	}

	// 以降の Move / 衝突 / 寿命処理は基底に任せる
	NpcBullet::Update();                                  // :contentReference[oaicite:4]{index=4}
}

void NpcHomingBullet::SetTarget(const BaseObject* target) { target_ = target; }
