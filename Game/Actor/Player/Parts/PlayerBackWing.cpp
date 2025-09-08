#include "PlayerBackWing.h"
#include "Easing/Easing.h"
#include "Frame/Frame.h"
#include "MathFunction.h"

void PlayerBackWing::Init(WorldTransform* transform, const std::string& GroupName) {
    // base初期化
    BasePlayerParts::Init(transform, GroupName);
}

void PlayerBackWing::Update() {
    float deltaTime = Frame::DeltaTime();

    BasePlayerParts::Update();

    // 入力による回転の線形補間
    inputRotation_ = Lerp(inputRotation_, targetInputRotation_, returnSpeed_ * deltaTime);

    // 回転設定
    obj3d_->transform_.rotation_.x = offsetRotate_.x + inputRotation_.x;
    obj3d_->transform_.rotation_.y = offsetRotate_.y + inputRotation_.y;
    obj3d_->transform_.rotation_.z = offsetRotate_.z + baseRotate_.z + inputRotation_.z;
}

void PlayerBackWing::AdjustParam() {
    BasePlayerParts::AdjustParam();
}