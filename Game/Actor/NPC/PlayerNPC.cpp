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
	if (isInitialized_) { return; }
	isInitialized_ = true;

	obj3d_.reset(Object3d::CreateModel("Enemy.obj")); //味方npcだけどいったんモデルを代用
	BaseObject::Init();
	obj3d_->transform_.parent_ = &baseTransform_;

	NPC::Init();
}

/// ===================================================
///  更新
/// ===================================================
void PlayerNPC::Update() {
	NPC::Update();
}
