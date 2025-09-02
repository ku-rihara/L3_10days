#pragma once

#include "BasePlayerBehavior.h"

class PlayerBoost : public BasePlayerBehavior {
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
