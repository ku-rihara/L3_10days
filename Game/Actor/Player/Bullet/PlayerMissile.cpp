#include "PlayerMissile.h"
// lockOn
#include "Actor/NPC/BoundaryBreaker/BoundaryBreaker.h"
#include "Actor/NPC/EnemyNPC.h"
#include "Actor/Player/Player.h"
#include "BasePlayerBullet.h"
#include "Frame/Frame.h"
#include "MathFunction.h"
#include "Matrix4x4.h"
#include "Physics/SweepAabb.h"
#include "PlayerBulletShooter.h"

#include "audio/Audio.h"

#include <numbers>


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
    currentSpeed_    = 0.0f;
    hasTarget_       = false;
    targetPosition_  = Vector3::ZeroVector();
    targetId_        = INVALID_TARGET_ID;
    targetManager_   = TargetManager::GetInstance();

    particleShooter_ = nullptr;
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

    // 速度更新
    UpdateSpeed(deltaTime);

    // ミサイルの移動処理
    UpdateMissileMovement(deltaTime);

    // 位置を更新
    baseTransform_.translation_ += velocity_ * deltaTime;

    HitBoundary();

    // トランスフォーム更新
    BaseObject::Update();
    cTransform_.translation_ = GetWorldPosition();
    cTransform_.UpdateMatrix();

    // パーティクル発射
    if (particleShooter_) {
        Quaternion qRot = baseTransform_.quaternion_.Normalize();
        Vector3 eRot    = qRot.ToEuler();
        Matrix4x4 matRot = MakeRotateMatrixQuaternion(qRot);
        Vector3 offsets  = {-0.0f, -0.0f, -1.0f};
      
        Vector3 EmitPos = (baseTransform_.GetWorldPos()) + TransformMatrix(offsets, matRot);

        particleShooter_->EmitMissileParticle(EmitPos,eRot);
    }
}

void PlayerMissile::UpdateSpeed(float deltaTime) {
    // 加速度による速度増加
    currentSpeed_ += uniqueParam_.acceleration * deltaTime;

    // 最高速度でクランプ
    currentSpeed_ = std::min(currentSpeed_, uniqueParam_.maxSpeed);
}

void PlayerMissile::UpdateMissileMovement(float deltaTime) {
    if (hasTarget_ && IsTargetValid()) {
        UpdateTargetTracking(deltaTime);
    }

    // 速度ベクトルを現在の速度に合わせて調整
    if (velocity_.Length() > 0.0f) {
        velocity_ = velocity_.Normalize() * currentSpeed_;
    }

    // 進行方向に基づいて姿勢を更新
    UpdateMissileOrientationFromVelocity();
}

void PlayerMissile::UpdateTargetTracking(float deltaTime) {
    if (!IsTargetValid()) {
        return;
    }

    Vector3 currentPos = baseTransform_.translation_;
    Vector3 targetPos  = GetTargetWorldPosition();

    Vector3 toTarget = targetPos - currentPos;
    if (toTarget.Length() < 0.1f) {
        isActive_ = false;
        return;
    }

    Vector3 desiredDirection = toTarget.Normalize();
    Vector3 currentDirection = velocity_.Normalize();

    float dot   = Vector3::Dot(currentDirection, desiredDirection);
    dot         = std::clamp(dot, -1.0f, 1.0f);
    float angle = std::acos(dot);

    float maxAngleChange = uniqueParam_.maxTurnRate * deltaTime;

    if (angle > maxAngleChange) {
        Vector3 axis = Vector3::Cross(currentDirection, desiredDirection).Normalize();
        if (axis.Length() < 0.001f) {
            axis = Vector3(0.0f, 1.0f, 0.0f);
        }

        Quaternion rotation  = Quaternion::MakeRotateAxisAngle(axis, maxAngleChange);
        Vector3 newDirection = rotation.RotateVector(currentDirection);

        Vector3 finalDirection = Lerp(currentDirection, newDirection, uniqueParam_.trackingStrength * deltaTime);
        velocity_              = finalDirection.Normalize() * currentSpeed_;
    } else {
        Vector3 finalDirection = Lerp(currentDirection, desiredDirection, uniqueParam_.trackingStrength * deltaTime);
        velocity_              = finalDirection.Normalize() * currentSpeed_;
    }
}

void PlayerMissile::UpdateMissileOrientationFromVelocity() {
    if (velocity_.Length() > 0.0f) {
        // 進行方向を基にクォータニオンを計算
        Vector3 forward = velocity_.Normalize();
        Vector3 up      = Vector3(0.0f, 1.0f, 0.0f); // 上方向（Y軸）

        // 進行方向と上方向から右方向を計算
        Vector3 right = Vector3::Cross(up, forward).Normalize();
        up            = Vector3::Cross(forward, right).Normalize();

        // 回転行列を作成
        Matrix4x4 rotationMatrix;
        rotationMatrix.m[0][0] = right.x;
        rotationMatrix.m[1][0] = right.y;
        rotationMatrix.m[2][0] = right.z;

        rotationMatrix.m[0][1] = up.x;
        rotationMatrix.m[1][1] = up.y;
        rotationMatrix.m[2][1] = up.z;

        rotationMatrix.m[0][2] = forward.x;
        rotationMatrix.m[1][2] = forward.y;
        rotationMatrix.m[2][2] = forward.z;

        // 回転行列をクォータニオンに変換
        baseTransform_.quaternion_ = Quaternion::FromMatrix(rotationMatrix);
    }
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

    // 初速を設定
    currentSpeed_ = param_.speed;
    velocity_     = player.GetForwardVector().Normalize() * currentSpeed_;

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
    uniqueParam_ = params;
}

void PlayerMissile::OnCollisionStay([[maybe_unused]] BaseCollider* other) {
    if (dynamic_cast<BoundaryBreaker*>(other) || dynamic_cast<EnemyNPC*>(other)) {
        Deactivate();
    }
}

void PlayerMissile::HitBoundary() {
    auto boundary = Boundary::GetInstance();

    Vector3 prevPos_    = baseTransform_.translation_ - velocity_ * Frame::DeltaTime();
    Vector3 currentPos_ = baseTransform_.GetWorldPos();

    if (boundary) {
        const AABB box = boundary->GetWorldAabb();
        auto hit       = Sweep::SegmentSphereVsAabb(prevPos_, baseTransform_.translation_, param_.collisionRadiusForBoundary, box);
        if (hit) {
            // 穴内なら無効
            if (!boundary->IsInHoleXZ(hit->point, param_.collisionRadiusForBoundary)) {
                // 破壊通知（AddCrack 内部呼び出し）
                if (boundary->OnBulletImpact(*hit, param_.damage * 2.0f)) {
					/// SEの再生
					Audio* audio = Audio::GetInstance();
					int se = audio->LoadWave("./resources/Sound/SE/BoundaryCollision.wav");
					audio->PlayWave(se, 0.1f);
                }
                Deactivate();
            }
        }
    }
}