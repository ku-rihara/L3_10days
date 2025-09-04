#pragma once

#include "Easing/Easing.h"
#include <memory>
#include <string>

class Player;
class Input;

class BasePlayerSpeedBehavior {
public:
    BasePlayerSpeedBehavior(const std::string& name, Player* player);
    virtual ~BasePlayerSpeedBehavior() {}

    virtual void Update()      = 0;
    virtual void Debug()       = 0;
    virtual void HandleInput() = 0;

    // Behavior切り替えのチェック
    virtual std::unique_ptr<BasePlayerSpeedBehavior> CheckForBehaviorChange() = 0;

    // 状態の引き継ぎ
    virtual void TransferStateFrom(BasePlayerSpeedBehavior* previousBehavior);

    // 速度とイージング時間の取得/設定
    virtual float GetCurrentSpeed() const       = 0;
    virtual float GetCurrentEaseTime() const    = 0;
    virtual void SetCurrentSpeed(float speed)   = 0;
    virtual void SetCurrentEaseTime(float time) = 0;

protected:
    // 振る舞い名
    std::string name_;
    // Player
    Player* pPlayer_ = nullptr;

    // 入力状態
    bool isLBPressed_  = false;
    bool wasLBPressed_ = false;

    // 各Behaviorで独自のEasingを持つ
    Easing<float> speedEase_;
    float currentSpeed_ = 0.0f;

    void UpdateInputState();
    Input* GetInput();
};