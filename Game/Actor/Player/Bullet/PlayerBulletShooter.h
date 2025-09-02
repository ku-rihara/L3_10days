#pragma once

#include "BasePlayerBullet.h"
#include "utility/ParameterEditor/GlobalParameter.h"
#include <array>
#include <cstdint>
#include <string>

struct ShooterParameter {
    float intervalTime;
    int32_t maxBulletNum;
    float reloadTime;
};

class PlayerBulletShooter {
public:
    PlayerBulletShooter()  = default;
    ~PlayerBulletShooter() = default;

public:
    void Init();

    ///-------------------------------------------------------------------------------------
    /// Editor
    ///-------------------------------------------------------------------------------------
    void BindParams();
    void AdjustParam();
    void DrawEnemyParamUI(BulletType type);

private:
    // globalParameter
    GlobalParameter* globalParameter_;
    const std::string groupName_ = "Bullets";
    std::array<BulletParameter, static_cast<int32_t>(BulletType::COUNT)> bulletParameters_;
    std::array<ShooterParameter, static_cast<int32_t>(BulletType::COUNT)> shooterParameters_;
    std::array <std::string, static_cast<int32_t>(BulletType::COUNT) > typeNames_;
};
