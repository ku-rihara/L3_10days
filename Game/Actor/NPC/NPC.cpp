#include "NPC.h"

#include "Actor/Station/Base/BaseStation.h"
#include "Actor/Boundary/Boundary.h"
#include "Frame/Frame.h"

#include <cmath>
#include <numbers>

namespace{
inline float WrapPi(float a){
	// [-pi, pi] に折り返し
	while (a > std::numbers::pi_v<float>) a -= 2.0f * std::numbers::pi_v<float>;
	while (a < -std::numbers::pi_v<float>) a += 2.0f * std::numbers::pi_v<float>;
	return a;
}

inline float MoveTowardsAngle(float current, float target, float maxStep){
	float d = WrapPi(target - current);
	if (std::fabs(d) <= maxStep) return current + d;
	return current + std::copysign(maxStep,d);
}
}

// ===== BoundaryHoleSource 実装 =====
const std::vector<Hole>& NPC::BoundaryHoleSource::GetHoles() const{
	static const std::vector<Hole> kEmpty;
	return boundary ? boundary->GetHoles() : kEmpty;
}

/// ===================================================
/// 初期化
/// ===================================================
void NPC::Init(){
	globalParam_ = GlobalParameter::GetInstance();

	// グループ名が入っていなければデフォルト
	if (groupName_.empty()){ groupName_ = "UnnamedNPC"; }

	// メイングループの設定
	globalParam_->CreateGroup(groupName_,true);

	// 重複バインドを防ぐ
	globalParam_->ClearBindingsForGroup(groupName_);
	BindParms();

	// パラメータ同期
	globalParam_->SyncParamForGroup(groupName_);

	// LoadData(); // 必要なら

	Activate();

	// 航法初期化（自分の位置を中心に旋回開始）
	navigator_.Reset(GetWorldPosition());

	// --- 通行制約（XZ 長方形＋穴ゲート）をセットアップ ---
	// あなたの Boundary メッシュは x,z ∈ [-1500,1500] の板なので、それを Rect に反映
	Boundary* boundary = Boundary::GetInstance();
	holeSource_.boundary = boundary;

	RectXZ rect{-1500.0f, 1500.0f, -1500.0f, 1500.0f};
	moveConstraint_ = std::make_unique<RectXZWithGatesConstraint>(&holeSource_,rect,0.01f);

	isInitialized_ = true;
}

/// ===================================================
/// 更新
/// ===================================================
void NPC::Update(){
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
    const Vector3 tgtPos = (target_) ? target_->GetWorldPosition() : npcPos;

    const Boundary* boundary = Boundary::GetInstance();
    const std::vector<Hole>& holes = boundary->GetHoles();

    if (speed_ != navConfig_.speed) {
        navConfig_.speed = speed_;
    }

    const Vector3 desiredDelta = navigator_.Tick(dt, npcPos, tgtPos, holes);

    Vector3 from = baseTransform_.translation_;
    Vector3 to   = from + desiredDelta;

    if (moveConstraint_) {
        to = moveConstraint_->FilterMove(from, to);
    }

    // === 進行方向に向けてオイラー回転 ===
    const Vector3 v = to - from;
    const float   vLen = v.Length();
    if (vLen > 1e-6f) {
        // 進行方向の正規化
        const Vector3 dir = v / vLen;

        // ヨー(Y): XZ 平面の向き（前を +Z と想定）
        const float targetYaw   = std::atan2(dir.x, dir.z);

        // ピッチ(X): 上下の向き（右手系 +Z forward を想定）
        // 俯角を正にしたいなら -dir.y、仰角を正なら +dir.y に合わせて調整
        const float targetPitch = std::atan2(-dir.y, std::sqrt(dir.x*dir.x + dir.z*dir.z));

        // 角速度上限（お好みでデータ駆動に）
        const float turnRateRadPerSec = std::numbers::pi_v<float> * 2.0f; // 360°/s
        const float maxStep = turnRateRadPerSec * dt;

        // WorldTransform は Euler 回転を使う
        baseTransform_.rotateOder_ = RotateOder::XYZ;

        // 現在角度を取得して補間
        Vector3& rot = baseTransform_.rotation_; // (x=pitch, y=yaw, z=roll)
        rot.y = MoveTowardsAngle(rot.y, targetYaw,   maxStep);
        rot.x = MoveTowardsAngle(rot.x, targetPitch, maxStep);

        // ---- 任意: バンク(ロール)を加える ----
        const float bankGain = 0.6f;                 // 係数（調整用）
        const float bankMax  = std::numbers::pi_v<float> * 0.35f; // 最大 ~63°
        // 目標ヨーとの差で左右判定（単純モデル）
        float yawErr = WrapPi(targetYaw - rot.y);
        float targetBank = std::clamp(-yawErr * bankGain, -bankMax, bankMax);
        // ロール(Z)をゆっくり追従
        const float bankRate = std::numbers::pi_v<float> * 1.2f; // 追従速度
        rot.z = MoveTowardsAngle(rot.z, targetBank, bankRate * dt);
        // -------------------------------------
    }

    // 位置の反映
    baseTransform_.translation_ = to;

    if (!target_) {
        StartOrbit(npcPos);
    }
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

	// globalParam_->Bind(groupName_, "arriveSlowRadius",  &navConfig_.arriveSlowRadius);
	// globalParam_->Bind(groupName_, "minHoleRadius",     &navConfig_.minHoleRadius);
	// globalParam_->Bind(groupName_, "passFrac",          &navConfig_.passFrac);
	// globalParam_->Bind(groupName_, "retargetInterval",  &navConfig_.retargetInterval);
	// globalParam_->Bind(groupName_, "orbitRadius",       &navConfig_.orbitRadius);
	// globalParam_->Bind(groupName_, "orbitAngularSpd",   &navConfig_.orbitAngularSpd);
}

void NPC::LoadData(){ globalParam_->LoadFile(groupName_,fileDirectory_); }
void NPC::SaveData(){ globalParam_->SaveFile(groupName_,fileDirectory_); }

/////////////////////////////////////////////////////////////////////////////////////////
//      accessor
/////////////////////////////////////////////////////////////////////////////////////////

void NPC::SetTarget(const BaseStation* target){ target_ = target; }
void NPC::SetFaction(FactionType faction){ faction_ = faction; }
void NPC::Activate(){ isActive_ = true; }
void NPC::Deactivate(){ isActive_ = false; }
