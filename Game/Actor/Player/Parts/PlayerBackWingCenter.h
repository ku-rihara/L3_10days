#pragma once
#include "BasePlayerParts.h"

class PlayerBackWingCenter : public BasePlayerParts {
public:
    PlayerBackWingCenter()     = default;
    ~PlayerBackWingCenter() override = default;

    void Init(WorldTransform* transform,const std::string& GroupName) override;
    void Update() override;

    void AdjustParam() override;

private:
};