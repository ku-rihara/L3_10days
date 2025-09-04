#pragma once

#include "BasePlayerSpeedBehavior.h"

class PlayerAccelerator : public BasePlayerSpeedBehavior {
private:
    
    /// ===================================================
    /// private variable
    /// ===================================================
 

public:
    // コンストラクタ
    PlayerAccelerator(Player* player);
    ~PlayerAccelerator();

    void Update() override;
    void Debug() override;
};
