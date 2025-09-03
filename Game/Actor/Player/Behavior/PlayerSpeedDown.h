#pragma once

#include "BasePlayerSpeedBehavior.h"

class PlayerSpeedDown : public BasePlayerSpeedBehavior {
private:
    /// ===================================================
    /// private variable
    /// ===================================================

public:
    // コンストラクタ
    PlayerSpeedDown(Player* player);
    ~PlayerSpeedDown();

    void Update() override;
    void Debug() override;
};
