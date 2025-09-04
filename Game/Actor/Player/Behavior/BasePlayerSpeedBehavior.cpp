#include "BasePlayerSpeedBehavior.h"
#include "Actor/Player/Player.h"
#include "input/Input.h"

BasePlayerSpeedBehavior::BasePlayerSpeedBehavior(const std::string& name, Player* player) {
    name_    = name;
    pPlayer_ = player;

    // Easingを個別に初期化
    speedEase_.Init(name + "Speed", name + "Speed.json");
    speedEase_.SetAdaptValue(&currentSpeed_);
    speedEase_.Reset();
    speedEase_.SetIsNoEnd(true);
}

void BasePlayerSpeedBehavior::TransferStateFrom(BasePlayerSpeedBehavior* previousBehavior) {
    if (previousBehavior) {
        // 前のBehaviorから現在の速度を取得
        float prevSpeed    = previousBehavior->GetCurrentSpeed();
        float prevEaseTime = previousBehavior->GetCurrentEaseTime();

        // 現在のBehaviorの開始値を前のBehaviorの現在値に設定
        speedEase_.SetStartValue(prevSpeed);
        currentSpeed_ = prevSpeed;
        speedEase_.SetCurrentValue(prevSpeed);
        speedEase_.SetCurrentEaseTime(prevEaseTime);

        // 入力状態も引き継ぐ
        wasLBPressed_ = previousBehavior->wasLBPressed_;
        isLBPressed_  = previousBehavior->isLBPressed_;
    }
}

void BasePlayerSpeedBehavior::UpdateInputState() {
    wasLBPressed_ = isLBPressed_;
    isLBPressed_  = Input::IsPressRightTrigger(0, 1);
}

Input* BasePlayerSpeedBehavior::GetInput() {
    return Input::GetInstance();
}