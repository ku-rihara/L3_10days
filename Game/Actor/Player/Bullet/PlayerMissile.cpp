#include "PlayerMissile.h"
// lockOn
#include "Actor/NPC/BoundaryBreaker/BoundaryBreaker.h"
#include "Actor/NPC/EnemyNPC.h"
#include "Actor/Player/Player.h"
#include "BasePlayerBullet.h"
#include "Frame/Frame.h"
#include "MathFunction.h"
#include "Matrix4x4.h"
#include "PlayerBulletShooter.h"
#include <numbers>
#include "Physics/SweepAabb.h"

void PlayerMissile::Init() {
    // モデル作成
    obj3d_.reset(Object3d::CreateModel("Missile.obj"));

    // transformの初期化
    baseTransform_.Init();
    baseTransform_.rotateOder_ = RotateOder::Quaternion;
    baseTransform_.quaternion_ = Quaternion::Identity();
    obj3d_->transform_.parent_ = &baseTransform_;

    // 初期値設定
    isActive_        = false;
    currentLifeTime_ = 0.0f;
    velocity_        = Vector3::ZeroVector();
    hasTarget_       = false;
    targetPosition_  = Vector3::ZeroVector();
    targetId_        = INVALID_TARGET_ID;
    targetManager_   = TargetManager::GetInstance();
}

void PlayerMissile::Update() {
    if (!isActive_) {
        return;
    }

    float deltaTime = Frame::DeltaTime();

    // 寿命チェック
    currentLifeTime_ += deltaTime;
    if (currentLifeTime_ >= param_.lifeTime) {
        isActive_ = false;
        return;
    }

    // ミサイルの移動処理
    UpdateMissileMovement(deltaTime);

    // 位置を更新
    baseTransform_.translation_ += velocity_ * deltaTime;

    HitBoundary();

    // トランスフォーム更新
    BaseObject::Update();
	cTransform_.translation_ = GetWorldPosition();
	cTransform_.UpdateMatrix();
}

void PlayerMissile::UpdateMissileMovement(float deltaTime) {
    if (hasTarget_ && IsTargetValid()) {
        UpdateTargetTracking(deltaTime);
    }

    // 速度の大きさを維持
    float currentSpeed = velocity_.Length();
    if (currentSpeed > 0.0f) {
        velocity_ = velocity_.Normalize() * param_.speed;
    }
}

void PlayerMissile::UpdateTargetTracking(float deltaTime) {
    if (!IsTargetValid()) {
        // ターゲットが無効になった場合、直進に切り替え
        return;
    }

    Vector3 currentPos = baseTransform_.translation_;
    Vector3 targetPos  = GetTargetWorldPosition();

    Vector3 toTarget = targetPos - currentPos;
    if (toTarget.Length() < 0.1f) {
        // ターゲットに到達
        isActive_ = false;
        return;
    }

    Vector3 desiredDirection = toTarget.Normalize();
    Vector3 currentDirection = velocity_.Normalize();

    float dot   = Vector3::Dot(currentDirection, desiredDirection);
    dot         = std::clamp(dot, -1.0f, 1.0f);
    float angle = std::acos(dot);

    float maxAngleChange = maxTurnRate_ * deltaTime;
    if (angle > maxAngleChange) {
        Vector3 axis = Vector3::Cross(currentDirection, desiredDirection).Normalize();
        if (axis.Length() < 0.001f) {
            axis = Vector3::ToUp();
        }
        Quaternion rotation  = Quaternion::MakeRotateAxisAngle(axis, maxAngleChange);
        Vector3 newDirection = rotation.RotateVector(currentDirection);

        Vector3 finalDirection = Lerp(currentDirection, newDirection, trackingStrength_ * deltaTime);
        velocity_              = finalDirection.Normalize() * param_.speed;
    } else {
        Vector3 finalDirection = Lerp(currentDirection, desiredDirection, trackingStrength_ * deltaTime);
        velocity_              = finalDirection.Normalize() * param_.speed;
    }

    // ミサイルの向きを移動方向に合わせる
    Matrix4x4 lookMatrix       = MakeRootAtMatrix(Vector3::ZeroVector(), velocity_.Normalize(), Vector3::ToUp());
    baseTransform_.quaternion_ = QuaternionFromMatrix(lookMatrix);
}

bool PlayerMissile::IsTargetValid() const {
    return targetManager_ && targetManager_->IsTargetValid(targetId_);
}

Vector3 PlayerMissile::GetTargetWorldPosition() const {
    if (IsTargetValid()) {
        return targetManager_->GetTargetPosition(targetId_);
    }
    return targetPosition_; // フォールバック
}

void PlayerMissile::Fire(const Player& player, const LockOn::LockOnVariant* target) {
    if (target && targetManager_) {
        // ターゲットをTargetManagerに登録してIDを取得
        targetId_  = targetManager_->RegisterTarget(*target);
        hasTarget_ = true;
    } else {
        targetId_  = INVALID_TARGET_ID;
        hasTarget_ = false;
    }

    // 発射位置を設定
    baseTransform_.translation_ = player.GetWorldPosition();

    // プレイヤーの回転
    baseTransform_.quaternion_     = player.GetBaseQuaternion();
    obj3d_->transform_.quaternion_ = player.GetObjQuaternion();

    // 速度を設定
    velocity_ = player.GetForwardVector().Normalize() * param_.speed;

    // 初期化
    currentLifeTime_ = 0.0f;
    isActive_        = true;
}

void PlayerMissile::SetTargetID(TargetID targetId) {
    targetId_  = targetId;
    hasTarget_ = (targetId != INVALID_TARGET_ID);
}

void PlayerMissile::SetTarget(const Vector3& targetPosition) {
    targetPosition_ = targetPosition;
    hasTarget_      = true;
    targetId_       = INVALID_TARGET_ID; 
}

void PlayerMissile::ClearTarget() {
    hasTarget_ = false;
    targetId_  = INVALID_TARGET_ID;
}

void PlayerMissile::Deactivate() {
    isActive_ = false;
}

Vector3 PlayerMissile::GetPosition() const {
    return baseTransform_.GetWorldPos();
}

// パラメータ設定
void PlayerMissile::SetMissileParameters(const MissileParameter& params) {
    trackingStrength_ = params.trackingStrength;
    maxTurnRate_      = params.maxTurnRate;
}


void PlayerMissile::HitBoundary() {
    auto boundary = Boundary::GetInstance();

    Vector3 prevPos_ = baseTransform_.translation_ - velocity_ * Frame::DeltaTime();
    Vector3 currentPos_ = baseTransform_.GetWorldPos();

    if (boundary) {
        const AABB box = boundary->GetWorldAabb();
        auto hit       = Sweep::SegmentSphereVsAabb(prevPos_, baseTransform_.translation_, param_.breakRadius, box);
        if (hit) {
            // 穴内なら無効
            if (!boundary->IsInHoleXZ(hit->point, param_.breakRadius)) {
                // 破壊通知（AddCrack 内部呼び出し）
                boundary->OnBulletImpact(*hit, param_.damage);
                Deactivate();
            }
        }
    }
}
