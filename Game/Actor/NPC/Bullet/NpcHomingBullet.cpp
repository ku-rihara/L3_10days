#include "NpcHomingBullet.h"
#include "Actor/ExpEmitter/ExpEmitter.h"
#include "Frame/Frame.h"
#include "utility/ParameterEditor/GlobalParameter.h"
#include <algorithm>
#include <cmath>

static inline float Clamp(float v, float lo, float hi) {
    return (v < lo) ? lo : (v > hi ? hi : v);
}

// 任意軸回転
static Vector3 RotateAroundAxis(const Vector3& v, const Vector3& axisN, float angleRad) {
    const float c = std::cos(angleRad), s = std::sin(angleRad);
    return v * c + Vector3::Cross(axisN, v) * s + axisN * (Vector3::Dot(axisN, v) * (1.0f - c));
}

// dir を desired へ最大 maxRad だけ回す（両方ノーマライズ想定）
static Vector3 RotateTowards(const Vector3& dir, const Vector3& desired, float maxRad) {
    Vector3 d0 = Vector3::Normalize(dir);
    Vector3 d1 = Vector3::Normalize(desired); // ← 修正: (desired, d0).Normalize() はバグ

    // -1..1 にクランプ（0..1 ではない！）
    float dot = Clamp(Vector3::Dot(d0, d1), -1.0f, 1.0f);
    float ang = std::acos(dot);
    if (ang <= 1e-6f)
        return d1; // ほぼ一致
    if (maxRad <= 0.0f)
        return d0; // 回せない

    // 目標角の方が小さければ一気に合わせる
    if (ang <= maxRad)
        return d1;

    // 回転軸
    Vector3 axis   = Vector3::Cross(d0, d1);
    float axisLen2 = Vector3::Dot(axis, axis);
    if (axisLen2 < 1e-12f) {
        // 平行 or 反平行：適当な直交軸を作る
        axis = (std::fabs(d0.y) < 0.99f)
                   ? Vector3::Normalize(Vector3::Cross(d0, {0, 1, 0}))
                   : Vector3::Normalize(Vector3::Cross(d0, {1, 0, 0}));
    } else {
        axis = axis * (1.0f / std::sqrt(axisLen2));
    }

    return Vector3::Normalize(RotateAroundAxis(d0, axis, maxRad));
}

void NpcHomingBullet::Init() {
    // 名前の初期化
    groupName_ = "NpcHomingBullet";
    modelName_ = "cube.obj";

    // パラメータとモデルの初期化
    NpcBullet::Init();

    if (turnRateDegPerSec_ <= 0.0f)
        turnRateDegPerSec_ = 360.0f; // 1秒で360°
    if (loseAngleDeg_ <= 0.0f)
        loseAngleDeg_ = 120.0f; // 後方120°超でロスト
    if (loseDistance_ <= 0.0f)
        loseDistance_ = 5000.0f; // ある程度遠距離まで追従
    // keepLockInsideCone_ は状況に応じて（false の方がロストしにくい）

    BindParms();
    if (globalParam_)
        globalParam_->SyncParamForGroup(groupName_);

    // 初速方向がゼロなら前方をセット（ゼロ方向だと回転もできない）
    if (Vector3::Dot(dir_, dir_) < 1e-8f) {
        dir_ = {0, 0, 1}; // エンジンの前方規約に合わせて
    } else {
        dir_ = Vector3::Normalize(dir_);
    }
}

void NpcHomingBullet::BindParms() {
    NpcBullet::BindParms();

    if (!globalParam_)
        return;
    globalParam_->Bind(groupName_, "turnRateDegPerSec", &turnRateDegPerSec_);
    globalParam_->Bind(groupName_, "loseAngleDeg", &loseAngleDeg_);
    globalParam_->Bind(groupName_, "loseDistance", &loseDistance_);
    globalParam_->Bind(groupName_, "keepLockInsideCone", &keepLockInsideCone_);
}

void NpcHomingBullet::SteerToTarget(float dt) {
    if (!target_)
        return;

    const Vector3 myPos = baseTransform_.translation_;
    const Vector3 toTgt = target_->GetWorldPosition() - myPos;
    const float dist    = toTgt.Length();

    // 距離条件：遠すぎたらロック解除
    if (dist > loseDistance_) {
        target_ = nullptr;
        return;
    }

    const Vector3 desired = (dist > 1e-6f) ? (toTgt * (1.0f / dist)) : dir_;

    // 現在角を計算
    float cosAng = Clamp(Vector3::Dot(Vector3::Normalize(dir_), desired), -1.0f, 1.0f);
    float angDeg = std::acos(cosAng) * 180.0f / 3.1415926535f;

    // 円錐外を即ロストするオプション（必要な場合のみ）
    if (keepLockInsideCone_ && angDeg > loseAngleDeg_) {
        target_ = nullptr;
        return;
    }

    // 最大角速度内で目標へ寄せる
    const float maxTurnRad = (turnRateDegPerSec_ * dt) * 3.1415926535f / 180.0f;
    dir_                   = RotateTowards(dir_, desired, maxTurnRad);
}

void NpcHomingBullet::Update() {
    if (!isActive_)
        return;

    const float dt = Frame::DeltaTime();

    // ターゲットがあるときのみ操舵
    if (target_) {
        SteerToTarget(dt);
    }

    // 実際の移動はベースクラスへ（dir_ を使って前進する想定）
    NpcBullet::Update();

    Quaternion qRot  = baseTransform_.quaternion_.Normalize();
    Vector3 eRot     = qRot.ToEuler();
    Matrix4x4 matRot = MakeRotateMatrixQuaternion(qRot);
    Vector3 offsets  = {-0.0f, -0.0f, -1.0f};
    Vector3 EmitPos  = (baseTransform_.GetWorldPos()) + TransformMatrix(offsets, matRot);
    ExpEmitter::GetInstance()->HormingEmitter(EmitPos, eRot);
}

void NpcHomingBullet::SetTarget(const BaseObject* target) {
    target_ = target;
}
