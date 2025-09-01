#include "EnemyNPC.h"

EnemyNPC::EnemyNPC() {
	//調整用グループ名
	groupName_ = "EnemyNPC";
	//派閥
	SetFaction(FactionType::Enemy);
}

/// ===================================================
///  初期化
/// ===================================================
void EnemyNPC::Init() {
	NPC::Init();
}

/// ===================================================
///  更新
/// ===================================================
void EnemyNPC::Update() {
	NPC::Update();
}
