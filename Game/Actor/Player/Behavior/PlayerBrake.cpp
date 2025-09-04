#include "PlayerBrake.h"
#include "Actor/Player/Player.h"
#include "Frame/Frame.h"
#include "PlayerAccelerator.h"
#include "PlayerAcceleUnattended.h"

PlayerBrake::PlayerBrake(Player* player)
    : BasePlayerSpeedBehavior("PlayerBrake", player) {
    InitializeEasing();
}

PlayerBrake::~PlayerBrake() {
}

void PlayerBrake::InitializeEasing() {
    // 加速用のイージングパラメータ設定
    speedEase_.SetEndValue(pPlayer_->GetSpeedParam().brakeForwardSpeed);
    // speedEase_.SetMaxTime(2.0f);
}

void PlayerBrake::ResetEasing() {
    speedEase_.SetStartValue(currentSpeed_);
    speedEase_.Reset();
}

void PlayerBrake::Update() {
    HandleInput();

    // Easing更新
    if (isLTPressed_) {
        speedEase_.Update(Frame::DeltaTime());
    }

    // Playerに速度を適用
    pPlayer_->SpeedUpdate();
}

void PlayerBrake::HandleInput() {
    UpdateInputState();
}

std::unique_ptr<BasePlayerSpeedBehavior> PlayerBrake::CheckForBehaviorChange() {
    UpdateInputState();

    // 加速する
    if (isRTPressed_ && !wasRTPressed_) {
        auto newBehavior = std::make_unique<PlayerAccelerator>(pPlayer_);
        newBehavior->TransferStateFrom(this);
        return newBehavior;
    }

     // 通常に戻す
    if (!isLTPressed_ && wasLTPressed_) {
        auto newBehavior = std::make_unique<PlayerAccelUnattended>(pPlayer_);
        newBehavior->TransferStateFrom(this);
        return newBehavior;
    }

    return nullptr;
}

float PlayerBrake::GetCurrentSpeed() const {
    return currentSpeed_;
}

float PlayerBrake::GetCurrentEaseTime() const {
    return speedEase_.GetCurrentEaseTime();
}

void PlayerBrake::SetCurrentSpeed(float speed) {
    currentSpeed_ = speed;
    speedEase_.SetCurrentValue(speed);
}

void PlayerBrake::SetCurrentEaseTime(float time) {
    speedEase_.SetCurrentEaseTime(time);
}

void PlayerBrake::Debug() {
}