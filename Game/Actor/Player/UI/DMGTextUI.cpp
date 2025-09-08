#include "DMGTextUI.h"
#include <imgui.h>
#include "base/TextureManager.h"

void DMGTextUI::Init(const std::string& fileName, const size_t& num) {
    BasePlayerUI::Init(fileName, num);

    // redLine Sprite
    uint32_t textureHandle = TextureManager::GetInstance()->LoadTexture(filePath_ + "Symbol.png");
    symbolSprite_.reset(Sprite::Create(textureHandle, Vector2::ZeroVector(), Vector4::kWHITE()));
    symbolSprite_->anchorPoint_ = Vector2(0.5f, 0.5f);
    symbolSprite_->SetUVScale(Vector2(0.2f, 1.0f));
    symbolSprite_->SetUVTranslate(Vector2(0.6f, 0.0f));
}

void DMGTextUI::Update() {

    // Position,Scale適応
    sprite_->SetPosition(basePosition_);
    sprite_->SetScale(baseScale_);

    symbolSprite_->SetPosition(symbolPos_);
    symbolSprite_->SetScale(symbolScale_);
}

void DMGTextUI::Draw() {
    sprite_->Draw();
    symbolSprite_->Draw();
}

void DMGTextUI::BindParams() {
    BasePlayerUI::BindParams();
    globalParameter_->Bind(groupName_, "symbolPos", &symbolPos_);
    globalParameter_->Bind(groupName_, "symbolScale", &symbolScale_);
}

void DMGTextUI::AdjustParam() {
    BasePlayerUI::AdjustParam();
   
}

void DMGTextUI::AdjustUniqueParam() {
    ImGui::DragFloat2("symbolPos", &symbolPos_.x, 0.01f);
    ImGui::DragFloat2("symbolScale", &symbolScale_.x, 0.01f);
}
