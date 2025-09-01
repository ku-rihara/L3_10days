#include "TestGround.h"

void TestGround::Init() {
    // モデル作成
    obj3d_.reset(Object3d::CreateModel("Field.obj"));
    // transform初期化
    baseTransform_.Init();
    obj3d_->transform_.parent_ = &baseTransform_;
    baseTransform_.scale_      = {100, 100, 100};
    baseTransform_.translation_.y = -10.0f;
}

void TestGround::Update() {

    BaseObject::Update();
}