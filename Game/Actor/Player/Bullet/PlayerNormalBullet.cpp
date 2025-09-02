#include "PlayerNormalBullet.h"
#include "Frame/Frame.h"
#include "MathFunction.h"
#include "Matrix4x4.h"
#include <numbers>

void PlayerNormalBullet::Init() {
    // モデル作成
    obj3d_.reset(Object3d::CreateModel("cube.obj"));

    // transformの初期化
    baseTransform_.Init();
    baseTransform_.rotateOder_ = RotateOder::Quaternion;
    baseTransform_.quaternion_ = Quaternion::Identity();
    obj3d_->transform_.parent_ = &baseTransform_;

    // 初期値設定
    isActive_        = false; 
    currentLifeTime_ = 0.0f;
    velocity_        = Vector3::ZeroVector();
}

void PlayerNormalBullet::Update() {
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

    UpdateNormalBullet(deltaTime);

    // 位置を更新
    baseTransform_.translation_ += velocity_ * deltaTime;

    // トランスフォーム更新
    BaseObject::Update();
}

void PlayerNormalBullet::UpdateNormalBullet(float deltaTime) {
    deltaTime;
}

void PlayerNormalBullet::Fire(const Vector3& position, const Vector3& direction, const Quaternion& rotation) {
    // 発射位置と向きを設定
    baseTransform_.translation_ = position;
    baseTransform_.quaternion_  = rotation;

    // 速度を設定（方向 × 速度）
    velocity_ = direction.Normalize() * param_.speed;

    // 弾丸を進行方向に向ける
    Matrix4x4 lookMatrix       = MakeRootAtMatrix(Vector3::ZeroVector(),direction, Vector3::ToUp());
    baseTransform_.quaternion_ = QuaternionFromMatrix(lookMatrix);

    // 初期化
    currentLifeTime_ = 0.0f;
    isActive_        = true;
}

void PlayerNormalBullet::Deactivate() {
    isActive_ = false;
}

Vector3 PlayerNormalBullet::GetPosition() const {
    return baseTransform_.GetWorldPos();
}