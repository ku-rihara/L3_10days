#include "Player.h"
#include "MathFunction.h"
#include <imgui.h>

void Player::Init() {

    // グローバルパラメータ
    globalParameter_ = GlobalParameter::GetInstance();
    globalParameter_->CreateGroup(groupName_, false);
    BindParams();
    globalParameter_->SyncParamForGroup(groupName_);

    // モデル作成
    obj3d_.reset(Object3d::CreateModel("Player.obj"));

    // transform初期化
    baseTransform_.Init();
    obj3d_->transform_.parent_ = &baseTransform_;
}

void Player::Update() {

    BaseObject::Update();
}

void Player::Move() {

}

///=========================================================
/// バインド
///==========================================================
void Player::BindParams() {
    globalParameter_->Bind(groupName_, "hp", &hp_);
    globalParameter_->Bind(groupName_, "speed", &speed_);
}

///=========================================================
/// パラメータ調整
///==========================================================
void Player::AdjustParam() {

#ifdef _DEBUG
    if (ImGui::CollapsingHeader(groupName_.c_str())) {
        ImGui::PushID(groupName_.c_str());

        ImGui::DragInt("Hp", &hp_);
        ImGui::DragFloat("speed", &speed_, 0.01f);

        // セーブ・ロード
        globalParameter_->ParamSaveForImGui(groupName_);
        globalParameter_->ParamLoadForImGui(groupName_);

        ImGui::PopID();
    }
#endif // _DEBUG
}

///==========================================================
/// 移動
///==========================================================
void Player::DirectionToCamera() {
    // 目標角度
    objectiveAngle_ = std::atan2(direction_.x, direction_.z);
    // 最短角度補間
    baseTransform_.rotation_.y = LerpShortAngle(baseTransform_.rotation_.y, objectiveAngle_, 0.3f);
}