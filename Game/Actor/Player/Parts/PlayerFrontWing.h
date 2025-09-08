#pragma once
#include "BasePlayerParts.h"

class PlayerFrontWing : public BasePlayerParts {
public:
    PlayerFrontWing()           = default;
    ~PlayerFrontWing() override = default;

    void Init(WorldTransform* transform,const std::string& GroupName) override;
    void Update() override;

    void AdjustParam() override;

private:
};