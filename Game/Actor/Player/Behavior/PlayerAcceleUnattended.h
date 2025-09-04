#pragma once

#include "BasePlayerSpeedBehavior.h"

class PlayerAccelUnattended : public BasePlayerSpeedBehavior {
public:
    PlayerAccelUnattended(Player* player);
    ~PlayerAccelUnattended();

    void Update() override;
    void Debug() override;
    void HandleInput() override;

    std::unique_ptr<BasePlayerSpeedBehavior> CheckForBehaviorChange() override;

    float GetCurrentSpeed() const override;
    float GetCurrentEaseTime() const override;
    void SetCurrentSpeed(float speed) override;
    void SetCurrentEaseTime(float time) override;

private:
    void InitializeEasing();
    void ResetEasing();
};
