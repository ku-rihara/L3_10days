#include "PlayerAcceleUnattended.h"
#include "Actor/Player/Player.h"
#include "Frame/Frame.h"
#include "PlayerAccelerator.h"
#include "PlayerBrake.h"

PlayerAccelUnattended::PlayerAccelUnattended(Player* player)
    : BasePlayerSpeedBehavior("PlayerAccelUnattended", player) {
    InitializeEasing();
}

PlayerAccelUnattended::~PlayerAccelUnattended() {
}

void PlayerAccelUnattended::InitializeEasing() {
    // 減速用のイージングパラメータ設定
    speedEase_.SetEndValue(pPlayer_->GetSpeedParam().minForwardSpeed);
}

void PlayerAccelUnattended::ResetEasing() {
    speedEase_.SetStartValue(currentSpeed_);
    speedEase_.Reset();
}

void PlayerAccelUnattended::Update() {
    HandleInput();

    // Easing更新
    speedEase_.Update(Frame::DeltaTime());

    // Playerに速度を適用
    pPlayer_->SpeedUpdate();
}

void PlayerAccelUnattended::HandleInput() {
    UpdateInputState();
}

std::unique_ptr<BasePlayerSpeedBehavior> PlayerAccelUnattended::CheckForBehaviorChange() {
    UpdateInputState();

    // PlayerAcceleratorに切り替え
    if (isRTPressed_ && !wasRTPressed_) {
        auto newBehavior = std::make_unique<PlayerAccelerator>(pPlayer_);
        newBehavior->TransferStateFrom(this);
        return newBehavior;
    }

      // ブレーキに切り替え
    if (isLTPressed_ && !wasLTPressed_) {
        auto newBehavior = std::make_unique<PlayerBrake>(pPlayer_);
        newBehavior->TransferStateFrom(this);
        return newBehavior;
    }

    return nullptr;
}

float PlayerAccelUnattended::GetCurrentSpeed() const {
    return currentSpeed_;
}

float PlayerAccelUnattended::GetCurrentEaseTime() const {
    return speedEase_.GetCurrentEaseTime();
}

void PlayerAccelUnattended::SetCurrentSpeed(float speed) {
    currentSpeed_ = speed;
    speedEase_.SetCurrentValue(speed);
}

void PlayerAccelUnattended::SetCurrentEaseTime(float time) {
    speedEase_.SetCurrentEaseTime(time);
}

void PlayerAccelUnattended::Debug() {
    // Debug implementation
}