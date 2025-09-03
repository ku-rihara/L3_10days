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
	if (isInitialized_) { return; }
	isInitialized_ = true;

	obj3d_.reset(Object3d::CreateModel("Enemy.obj")); //味方npcだけどいったんモデルを代用
	BaseObject::Init();
	obj3d_->transform_.parent_ = &baseTransform_;

	NPC::Init();
	SetColor(Vector4(1.0f, 0.0f, 0.0f, 1.0f));
}

/// ===================================================
///  更新
/// ===================================================
void EnemyNPC::Update() {
	NPC::Update();
}
