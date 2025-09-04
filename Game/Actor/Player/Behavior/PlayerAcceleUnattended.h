#pragma once

#include "BasePlayerSpeedBehavior.h"

class PlayerAcceleUnattended : public BasePlayerSpeedBehavior {
private:
    /// ===================================================
    /// private variable
    /// ===================================================

public:
    // コンストラクタ
    PlayerAcceleUnattended(Player* player);
    ~PlayerAcceleUnattended();

    void Update() override;
    void Debug() override;
};
