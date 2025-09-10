#include "PlayerLifeUI.h"
#include "Actor/Player/Player.h"
#include "Frame/Frame.h"
#include <imgui.h>

void PlayerLifeUI::Init(const std::string& fileName, const size_t& num) {
    BasePlayerUI::Init(fileName,num);
    EasingSet();
}

void PlayerLifeUI::Update() {

    // HP割合からLifeStep更新
    UpdateLifeStep();

    // Alpha,Scaleイージング更新
    if (lifeStep_ == LifeStep::DAUNGER) {
        alphaEase_->Update(Frame::DeltaTime());
        scaleEase_->Update(Frame::DeltaTime());
    } else {
        alpha_     = 1.0f;
        tempScale_ = Vector2::ZeroVector();
    }

    // Position,Scale適応
    sprite_->SetPosition(basePosition_);
    sprite_->SetScale(baseScale_ + tempScale_);

    // steColor,Alpha
    sprite_->SetColor(colorByLifeStep_[static_cast<size_t>(lifeStep_)]);
    sprite_->SetAlpha(alpha_);
}

void PlayerLifeUI::Draw() {
    sprite_->Draw();
}

void PlayerLifeUI::UpdateLifeStep() {

    float maxHp = pPlayer_->GetMaxHP();
    float hp    = pPlayer_->GetHP();

    float ratio = (hp / maxHp) * 100.0f;

    if (ratio > lifeStepPar_[static_cast<size_t>(LifeStep::FULL)]) {
        lifeStep_ = LifeStep::FULL;
    } else if (ratio > lifeStepPar_[static_cast<size_t>(LifeStep::MIDDLE)]) {
        lifeStep_ = LifeStep::MIDDLE;
    } else {
        lifeStep_ = LifeStep::DAUNGER;
    }
}

void PlayerLifeUI::EasingSet() {

    // alphaEase
    alphaEase_ = std::make_unique<Easing<float>>();
    alphaEase_->Init("PlayerIConAlpha", "PlayerIConAlpha.json");
    alphaEase_->SetAdaptValue(&alpha_);
    alphaEase_->SetOnFinishCallback([this]() {
        alphaEase_->Reset();
    });

    // ScaleEase
    scaleEase_ = std::make_unique<Easing<Vector2>>();
    scaleEase_->Init("PlayerIConScale", "PlayerIConScale.json");
    scaleEase_->SetAdaptValue(&tempScale_);
    scaleEase_->SetOnFinishCallback([this]() {
        scaleEase_->Reset();
    });
}

void PlayerLifeUI::BindParams() {
    BasePlayerUI::BindParams();

    for (size_t i = 0; i < lifeStepPar_.size(); ++i) {
        std::string name = ToString(i);
        globalParameter_->Bind(groupName_, "lifeStepPar" + name, &lifeStepPar_[i]);
        globalParameter_->Bind(groupName_, "colorByLifeStep" + name, &colorByLifeStep_[i]);
    }
}

void PlayerLifeUI::AdjustParam() {
    BasePlayerUI::AdjustParam();
}

void PlayerLifeUI::AdjustUniqueParam() {
    for (size_t i = 0; i < lifeStepPar_.size(); ++i) {
        std::string name = ToString(i);
        ImGui::DragFloat(("lifeStepPar" + name).c_str(), &lifeStepPar_[i], 0.01f);
    }

    for (size_t i = 0; i < lifeStepPar_.size(); ++i) {
        std::string name = ToString(i);
        ImGui::ColorEdit3(("colorByLifeStep" + name).c_str(), &colorByLifeStep_[i].x);
    }
}

std::string PlayerLifeUI::ToString(const size_t& stepIndex) {
    switch (stepIndex) {
    case 0:
        return "FULL";
    case 1:
        return "MIDDLE";
    case 2:
        return "DAUNGER";
    case 3:
        return "COUNT";
    default:
        return "UNKNOWN";
    }
}
