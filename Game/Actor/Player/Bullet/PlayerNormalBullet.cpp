#include "PlayerNormalBullet.h"
#include "Frame/Frame.h"
#include "Actor/Player/Player.h"
#include "BasePlayerBullet.h"

void PlayerNormalBullet::Init() {
    // モデル作成
    obj3d_.reset(Object3d::CreateModel("Bullet.obj"));

    // transformの初期化
    baseTransform_.Init();
    baseTransform_.rotateOder_ = RotateOder::Quaternion;
    baseTransform_.quaternion_ = Quaternion::Identity();

    obj3d_->transform_.rotateOder_ = RotateOder::Quaternion;
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

void PlayerNormalBullet::Fire(const Player& player, const LockOn::LockOnVariant* target) {
   
    // ターゲットを取得
    if (target) {
        target_ = target;
    }

    // 発射位置を設定
    baseTransform_.translation_ = player.GetWorldPosition();

    // プレイヤーの回転
    baseTransform_.quaternion_ = player.GetBaseQuaternion();
    obj3d_->transform_.quaternion_ = player.GetObjQuaternion();

    // 速度を設定
    velocity_ = player.GetForwardVector().Normalize() * param_.speed;

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