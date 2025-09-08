#include "BasePlayerUI.h"
#include "base/TextureManager.h"
#include <imgui.h>
#include <Vector4.h>

void BasePlayerUI::Init(const std::string& fileName) {
    groupName_ = fileName;
    // globalParameter
    globalParameter_ = GlobalParameter::GetInstance();
    globalParameter_->CreateGroup(groupName_, false);
    BindParams();
    globalParameter_->SyncParamForGroup(groupName_);

    // テクスチャ読み込み
    uint32_t textureHandle = TextureManager::GetInstance()->LoadTexture(filePath_ + fileName + ".png");
    sprite_.reset(Sprite::Create(textureHandle, Vector2::ZeroVector(), Vector4::kWHITE()));
    sprite_->anchorPoint_ = Vector2(0.5f, 0.5f);
}

///==========================================================
/// バインド
///==========================================================
void BasePlayerUI::BindParams() {
    globalParameter_->Bind(groupName_, "basePosition", &basePosition_);
    globalParameter_->Bind(groupName_, "baseScale", &baseScale_);
}

///==========================================================
/// パラメータ調整
///==========================================================
void BasePlayerUI::AdjustParam() {

#ifdef _DEBUG
    if (ImGui::CollapsingHeader(groupName_.c_str())) {
        ImGui::PushID(groupName_.c_str());

        ImGui::DragFloat2("basePosition", &basePosition_.x, 0.5f);
        ImGui::DragFloat2("baseScale", &baseScale_.x, 0.01f);

        AdjustUniqueParam();

        // セーブ・ロード
        globalParameter_->ParamSaveForImGui(groupName_);
        globalParameter_->ParamLoadForImGui(groupName_);

        ImGui::PopID();
    }
#endif // _DEBUG
}

void BasePlayerUI::SetPlayer(Player* player) {
    pPlayer_ = player;
  }