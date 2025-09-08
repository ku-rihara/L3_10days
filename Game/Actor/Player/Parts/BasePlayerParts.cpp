#include "BasePlayerParts.h"
#include "Easing/Easing.h"
#include "Frame/Frame.h"
#include <imgui.h>

void BasePlayerParts::Init(WorldTransform* transform, const std::string& GroupName) {
    groupName_ = GroupName;
    // グローバルパラメータ
    globalParameter_ = GlobalParameter::GetInstance();
    globalParameter_->CreateGroup(groupName_, false);
    BindParams();
    globalParameter_->SyncParamForGroup(groupName_);

    BaseObject::Init();
    obj3d_.reset(Object3d::CreateModel(groupName_ + ".obj"));
    obj3d_->transform_.parent_     = transform;
    obj3d_->transform_.rotateOder_ = RotateOder::XYZ;

    // 入力回転の初期化
    inputRotation_       = Vector3::ZeroVector();
    targetInputRotation_ = Vector3::ZeroVector();
}

void BasePlayerParts::Update() {
    // ポジション設定
    obj3d_->transform_.translation_ = offsetPos_;
    obj3d_->transform_.scale_       = scale_;
}

///=========================================================
/// バインド
///==========================================================
void BasePlayerParts::BindParams() {
    globalParameter_->Bind(groupName_, "offsetPos", &offsetPos_);
    globalParameter_->Bind(groupName_, "inputRotationSpeed", &inputRotationSpeed_);
    globalParameter_->Bind(groupName_, "returnSpeed", &returnSpeed_);
    globalParameter_->Bind(groupName_, "scale", &scale_);
    globalParameter_->Bind(groupName_, "offsetRotate", &offsetRotate_);
}

///=========================================================
/// パラメータ調整
///==========================================================
void BasePlayerParts::AdjustParam() {

#ifdef _DEBUG
    if (ImGui::CollapsingHeader(groupName_.c_str())) {
        ImGui::PushID(groupName_.c_str());

        ImGui::DragFloat3("offsetPos", &offsetPos_.x, 0.01f);
        ImGui::DragFloat3("Rotate", &offsetRotate_.x, 0.01f);
        ImGui::DragFloat3("scale", &scale_.x,0.01f);

        // 入力制御パラメータ
        ImGui::Separator();
        ImGui::Text("Input Control");
        ImGui::DragFloat("inputRotationSpeed", &inputRotationSpeed_, 0.1f, 0.1f, 10.0f);
        ImGui::DragFloat("returnSpeed", &returnSpeed_, 0.1f, 0.1f, 10.0f);

        // デバッグ情報
        ImGui::Separator();
        ImGui::Text("Debug Info");
        ImGui::Text("inputRotation: (%.2f, %.2f, %.2f)", inputRotation_.x, inputRotation_.y, inputRotation_.z);
        ImGui::Text("targetInputRotation: (%.2f, %.2f, %.2f)", targetInputRotation_.x, targetInputRotation_.y, targetInputRotation_.z);

        // セーブ・ロード
        globalParameter_->ParamSaveForImGui(groupName_);
        globalParameter_->ParamLoadForImGui(groupName_);

        ImGui::PopID();
    }
#endif // _DEBUG
}