#include "PlayerNPC.h"

PlayerNPC::PlayerNPC() {
	//調整用グループ名
	groupName_ = "PlayerNPC";
	//派閥
	SetFaction(FactionType::Ally);
}

/// ===================================================
///  初期化
/// ===================================================
void PlayerNPC::Init() {
	NPC::Init();
}

/// ===================================================
///  更新
/// ===================================================
void PlayerNPC::Update() {
	NPC::Update();
}
