#include "PlayerAccelerator.h"
#include "Actor/Player/Player.h"
#include "Frame/Frame.h"
#include "PlayerAcceleUnattended.h"
#include "PlayerBrake.h"

PlayerAccelerator::PlayerAccelerator(Player* player)
    : BasePlayerSpeedBehavior("PlayerAccelerator", player) {
    InitEasing();
}

PlayerAccelerator::~PlayerAccelerator() {
}

void PlayerAccelerator::InitEasing() {
    // 加速用のイージングパラメータ設定
    speedEase_.SetEndValue(pPlayer_->GetSpeedParam().maxForwardSpeed);
}

void PlayerAccelerator::ResetEasing() {
    speedEase_.SetStartValue(currentSpeed_);
    speedEase_.Reset();
}

void PlayerAccelerator::Update() {
    HandleInput();

    // Easing更新
    speedEase_.Update(Frame::DeltaTime());

    // Playerに速度を適用
    pPlayer_->SpeedUpdate();
}

void PlayerAccelerator::HandleInput() {
    UpdateInputState();
}

std::unique_ptr<BasePlayerSpeedBehavior> PlayerAccelerator::CheckForBehaviorChange() {
    UpdateInputState();

    // PlayerAccelUnattendedに切り替え
    if (!isRTPressed_ && wasRTPressed_) {
        auto newBehavior = std::make_unique<PlayerAccelUnattended>(pPlayer_);
        newBehavior->TransferStateFrom(this);
        return newBehavior;
    }

    if (isLTPressed_ && !wasLTPressed_) {
        // ブレーキに切り替え
        auto newBehavior = std::make_unique<PlayerBrake>(pPlayer_);
        newBehavior->TransferStateFrom(this);
        return newBehavior;
    }

    return nullptr;
}

float PlayerAccelerator::GetCurrentSpeed() const {
    return currentSpeed_;
}

float PlayerAccelerator::GetCurrentEaseTime() const {
    return speedEase_.GetCurrentEaseTime();
}

void PlayerAccelerator::SetCurrentSpeed(float speed) {
    currentSpeed_ = speed;
    speedEase_.SetCurrentValue(speed);
}

void PlayerAccelerator::SetCurrentEaseTime(float time) {
    speedEase_.SetCurrentEaseTime(time);
}

void PlayerAccelerator::Debug() {
}