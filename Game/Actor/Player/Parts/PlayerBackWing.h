#pragma once
#include "BasePlayerParts.h"

class PlayerBackWing : public BasePlayerParts {
public:
    PlayerBackWing()           = default;
    ~PlayerBackWing() override = default;

    void Init(WorldTransform* transform,const std::string& GroupName) override;
    void Update() override;

    void AdjustParam() override;

private:
};