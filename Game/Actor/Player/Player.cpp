#include "Player.h"

void Player::Init() {
    // モデル作成
    obj3d_.reset(Object3d::CreateModel("Suzanne.obj"));
    // transform初期化
    baseTransform_.Init();
    obj3d_->transform_.parent_ = &baseTransform_;
}

void Player::Update() {

    BaseObject::Update();
}