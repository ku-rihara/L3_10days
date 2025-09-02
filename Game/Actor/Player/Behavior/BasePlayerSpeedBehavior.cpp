#include "BasePlayerSpeedBehavior.h"
#include"Actor/Player/Player.h"


BasePlayerSpeedBehavior::BasePlayerSpeedBehavior(const std::string& name, Player* player) {
    name_             = name;
    pPlayer_          = player;
   /* pPlayerParameter_ = player->GetParameter();*/
}