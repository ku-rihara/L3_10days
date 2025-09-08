#include "MissileIconUI.h"
#include "Actor/Player/Bullet/PlayerBulletShooter.h"
#include "Actor/Player/Player.h"
#include "base/TextureManager.h"
#include "Frame/Frame.h"
#include "MathFunction.h"
#include <imgui.h>

void MissileIconUI::Init(const std::string& fileName, const size_t& num) {
    BasePlayerUI::Init(fileName, num);
    index_ = num;

    // redLine Sprite
    uint32_t textureHandle = TextureManager::GetInstance()->LoadTexture(filePath_ + "NoMissileUI.png");
    redLineSprite_.reset(Sprite::Create(textureHandle, Vector2::ZeroVector(), Vector4::kWHITE()));
    redLineSprite_->anchorPoint_ = Vector2(0.5f, 0.5f);
    // easingSet
    EasingSet();
}

void MissileIconUI::Update() {

    UpdateAbleShot();

    //
    if (isCollEndEase_) {
        scaleEase_->Update(Frame::DeltaTime());
    }

    // Position,Scale適応
    sprite_->SetPosition(basePosition_);
    sprite_->SetScale(baseScale_ + tempScale_);

    redLineSprite_->SetPosition(basePosition_);
    redLineSprite_->SetScale(redLineScale_);

    // Alpha
    redLineSprite_->SetAlpha(redLineAlpha_);
}

void MissileIconUI::UpdateAbleShot() {
    bool isAbleShot = pPlayer_->GetBulletShooter()->IsMissileSlotAvailable(int32_t(index_));

    // クールタイム発動
    if (!isAbleShot && !isColling_) {
        CollingStart();
    }
    // クーリング中でなければ抜ける
    if (!isColling_) {
        redLineAlpha_ = 0.0f;
        return;
    }

    float currentTime = pPlayer_->GetBulletShooter()->GetMissileSlotRemainingCooldown(int32_t(index_));
    float maxTime     = pPlayer_->GetBulletShooter()->GetMissileCollTimeMax();

    // 安全チェック
    if (maxTime <= 0.0f) {
        redLineAlpha_ = 0.0f;
        return;
    }

    // 0.0 ～ 1.0 の割合
    float t = currentTime / maxTime;
    t       = std::clamp(t, 0.0f, 1.0f);

    redLineAlpha_ = Lerp(endAlpha_, 1.0f, t);

    // 終了
    if (redLineAlpha_ <= endAlpha_) {
        isColling_     = false;
        isCollEndEase_ = true;
        scaleEase_->Reset();
    }
}

void MissileIconUI::Draw() {
    sprite_->Draw();
    if (isColling_) {
        redLineSprite_->Draw();
    }
}

void MissileIconUI::EasingSet() {
    // alphaEase
    scaleEase_ = std::make_unique<Easing<Vector2>>();
    scaleEase_->Init("MissileIconScale", "MissileIconScale.json");
    scaleEase_->SetAdaptValue(&tempScale_);
    scaleEase_->SetOnFinishCallback([this]() {
        isCollEndEase_ = false;
    });
    isColling_ = false;
}

void MissileIconUI::CollingStart() {
    isColling_ = true;
}
void MissileIconUI::BindParams() {
    BasePlayerUI::BindParams();
    globalParameter_->Bind(groupName_, "redLineScale", &redLineScale_);
    globalParameter_->Bind(groupName_, "endAlpha", &endAlpha_);
}

void MissileIconUI::AdjustParam() {
    BasePlayerUI::AdjustParam();
}

void MissileIconUI::AdjustUniqueParam() {
    ImGui::DragFloat("endAlpha", &endAlpha_, 0.01f);
    ImGui::DragFloat2("redLineScale", &redLineScale_.x, 0.01f);
}
