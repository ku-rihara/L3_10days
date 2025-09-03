#pragma once

#include <string>

class Player;
class BasePlayerSpeedBehavior {
protected:
    // 振る舞い名
    std::string name_;
    // Player
    Player* pPlayer_ = nullptr;
   
public:

    BasePlayerSpeedBehavior(const std::string& name, Player* player);
    virtual ~BasePlayerSpeedBehavior() {}

    virtual void Update() = 0;
    virtual void Debug() = 0;
};