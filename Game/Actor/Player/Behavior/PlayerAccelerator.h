#pragma once

#include "BasePlayerSpeedBehavior.h"
#include "Easing/Easing.h"

class PlayerAccelerator : public BasePlayerSpeedBehavior {
public:
    PlayerAccelerator(Player* player);
    ~PlayerAccelerator();

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
