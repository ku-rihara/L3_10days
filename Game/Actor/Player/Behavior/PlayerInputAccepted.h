#pragma once

#include "BasePlayerBehavior.h"

class PlayerInputAccepted : public BasePlayerBehavior {
private:
    /// ===================================================
    /// private variable
    /// ===================================================

public:
    // コンストラクタ
    PlayerInputAccepted(Player* player);
    ~PlayerInputAccepted();

    void Update() override;
    void Debug() override;
};
