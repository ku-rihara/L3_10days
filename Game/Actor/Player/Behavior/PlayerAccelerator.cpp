#include "PlayerAccelerator.h"
#include "Actor/Player/Player.h"
#include "Frame/Frame.h"
#include "PlayerAcceleUnattended.h"

PlayerAccelerator::PlayerAccelerator(Player* player)
    : BasePlayerSpeedBehavior("PlayerAccelerator", player) {
    InitializeEasing();
}

PlayerAccelerator::~PlayerAccelerator() {
}

void PlayerAccelerator::InitializeEasing() {
    // 加速用のイージングパラメータ設定
    speedEase_.SetEndValue(100.0f); // 最大速度
    speedEase_.SetMaxTime(2.0f); // 加速時間
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

    // ボタンが離された場合、PlayerAccelUnattendedに切り替え
    if (!isLBPressed_ && wasLBPressed_) {
        auto newBehavior = std::make_unique<PlayerAccelUnattended>(pPlayer_);
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
    // Debug implementation
}