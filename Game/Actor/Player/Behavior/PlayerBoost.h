#pragma once

#include "BasePlayerSpeedBehavior.h"

class PlayerBoost : public BasePlayerSpeedBehavior {
private:
    
    /// ===================================================
    /// private variable
    /// ===================================================
 

public:
    // コンストラクタ
    PlayerBoost(Player* player);
    ~PlayerBoost();

    void Update() override;
    void Debug() override;
};
