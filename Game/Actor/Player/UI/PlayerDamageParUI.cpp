#include "PlayerDamageParUI.h"
#include "base/TextureManager.h"
#include"Actor/Player/Player.h"
#include <imgui.h>

void PlayerDamageParUI::Init(const std::string& fileName, const size_t& num) {
    groupName_ = fileName;
    num;

    // globalParameter
    globalParameter_ = GlobalParameter::GetInstance();
    globalParameter_->CreateGroup(groupName_, false);
    BindParams();
    globalParameter_->SyncParamForGroup(groupName_);

    // 数字のテクスチャ読み込み（0-9の連番）
    uint32_t digitTextureHandle = TextureManager::GetInstance()->LoadTexture(filePath_ + "Numbers.png");

    // 各桁のスプライト初期化
    for (int i = 0; i < MAX_DIGITS; ++i) {
        digitSprites_[i].reset(Sprite::Create(digitTextureHandle, Vector2::ZeroVector(), Vector4::kWHITE()));
        digitSprites_[i]->anchorPoint_ = Vector2(0.5f, 0.5f);
        digitSprites_[i]->SetUVScale(Vector2(0.1f, 1.0f)); // 10個の数字があるので0.1
        SetDigitUV(digitSprites_[i], 0); // 初期値は0
    }

    // %マークのスプライト初期化
    uint32_t percentTextureHandle = TextureManager::GetInstance()->LoadTexture(filePath_ + "Symbol.png");
    percentSprite_.reset(Sprite::Create(percentTextureHandle, Vector2::ZeroVector(), Vector4::kWHITE()));
    percentSprite_->anchorPoint_ = Vector2(0.5f, 0.5f);
    percentSprite_->SetUVScale(Vector2(0.2f, 1.0f));
    percentSprite_->SetUVTranslate(Vector2(0.0f, 0.0f));
}

void PlayerDamageParUI::Update() {
   
    // ダメージパーセントを計算して桁スプライトを更新
    UpdateDigitSprites();

    // 数字スプライトの位置とスケール設定
    for (int i = 0; i < MAX_DIGITS; ++i) {
        Vector2 digitPos = basePosition_ + Vector2(digitSpacing_.x * i, digitSpacing_.y * i);
        digitSprites_[i]->SetPosition(digitPos);
        digitSprites_[i]->SetScale(baseScale_ * digitScale_);
    }

    // %マークの位置とスケール設定
    Vector2 percentPos = basePosition_ + percentOffset_;
    percentSprite_->SetPosition(percentPos);
    percentSprite_->SetScale(baseScale_ * percentScale_);
}

void PlayerDamageParUI::Draw() {
   

    // 数字を描画
    for (int i = 0; i < MAX_DIGITS; ++i) {
        digitSprites_[i]->Draw();
    }

    // %マークを描画
    percentSprite_->Draw();
}

void PlayerDamageParUI::BindParams() {
    BasePlayerUI::BindParams();
    globalParameter_->Bind(groupName_, "digitSpacing", &digitSpacing_);
    globalParameter_->Bind(groupName_, "percentOffset", &percentOffset_);
    globalParameter_->Bind(groupName_, "digitScale", &digitScale_);
    globalParameter_->Bind(groupName_, "percentScale", &percentScale_);
}

void PlayerDamageParUI::AdjustParam() {
    BasePlayerUI::AdjustParam();
}

void PlayerDamageParUI::AdjustUniqueParam() {
    ImGui::DragFloat2("digitSpacing", &digitSpacing_.x, 0.5f);
    ImGui::DragFloat2("percentOffset", &percentOffset_.x, 0.5f);
    ImGui::DragFloat2("digitScale", &digitScale_.x, 0.01f);
    ImGui::DragFloat2("percentScale", &percentScale_.x, 0.01f);
}

void PlayerDamageParUI::UpdateDigitSprites() {
    int damagePercent = CalculateDamagePercentage();

    // 各桁の数字を計算
    int hundreds = (damagePercent / 100) % 10;
    int tens     = (damagePercent / 10) % 10;
    int ones     = damagePercent % 10;

    // UV座標を設定
    SetDigitUV(digitSprites_[0], hundreds);
    SetDigitUV(digitSprites_[1], tens);
    SetDigitUV(digitSprites_[2], ones);

    // 100未満の場合は百の位を非表示にする
    if (damagePercent < 100) {
        digitSprites_[0]->SetAlpha(0.0f); // アルファを0にして非表示
    } else {
        digitSprites_[0]->SetAlpha(1.0f);
    }

    // 10未満の場合は十の位も非表示にする
    if (damagePercent < 10) {
        digitSprites_[1]->SetAlpha(0.0f);
    } else {
        digitSprites_[1]->SetAlpha(1.0f);
    }
}

void PlayerDamageParUI::SetDigitUV(std::unique_ptr<Sprite>& sprite, int digit) {
    // 0-9の数字に対応するUV座標を設定
    float uvX = digit * 0.1f; // 10個の数字があるので0.1間隔
    sprite->SetUVTranslate(Vector2(uvX, 0.0f));
}

int PlayerDamageParUI::CalculateDamagePercentage() const {
    if (!pPlayer_) {
        return 0;
    }

    float currentHP = pPlayer_->GetHP();
    float maxHP     = pPlayer_->GetMaxHP();

    if (maxHP <= 0.0f) {
        return 0;
    }

    // ダメージ率を計算（HP減少分の割合）
    float damageRatio = (maxHP - currentHP) / maxHP;
    int damagePercent = static_cast<int>(damageRatio * 100.0f);

    // 0-100の範囲にクランプ
    return (std::max)(0, (std::min)(100, damagePercent));
}