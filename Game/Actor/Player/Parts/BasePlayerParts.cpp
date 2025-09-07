#include "BasePlayerParts.h"
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
    obj3d_->transform_.parent_ = transform;
    obj3d_->transform_.rotateOder_ = RotateOder::Quaternion;
}

///=========================================================
/// バインド
///==========================================================
void BasePlayerParts::BindParams() {
    globalParameter_->Bind(groupName_, "offsetPos", &offsetPos_);
}

///=========================================================
/// パラメータ調整
///==========================================================
void BasePlayerParts::AdjustParam() {

#ifdef _DEBUG
    if (ImGui::CollapsingHeader(groupName_.c_str())) {
        ImGui::PushID(groupName_.c_str());

        ImGui::DragFloat3("offsetPos", &offsetPos_.x,0.01f);

        // セーブ・ロード
        globalParameter_->ParamSaveForImGui(groupName_);
        globalParameter_->ParamLoadForImGui(groupName_);

        ImGui::PopID();
    }
#endif // _DEBUG
}