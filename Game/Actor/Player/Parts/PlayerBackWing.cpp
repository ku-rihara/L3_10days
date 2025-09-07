#include "PlayerBackWing.h"

void PlayerBackWing::Init(WorldTransform* transform,const std::string& GroupName) {
    // base初期化
    BasePlayerParts::Init(transform,GroupName);
}

void PlayerBackWing::Update() {
    obj3d_->transform_.translation_ = offsetPos_;
}

void PlayerBackWing::AdjustParam() {
    BasePlayerParts::AdjustParam();
}
