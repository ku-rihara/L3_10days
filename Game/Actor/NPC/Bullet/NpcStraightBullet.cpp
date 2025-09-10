#include "NpcStraightBullet.h"
#include "utility/ParameterEditor/GlobalParameter.h"

// ===================================================
// 初期化
// ===================================================
void NpcStraightBullet::Init() {
	if (isInitialized_) return;

	modelName_ = "Missile.obj";
	groupName_ = "NpcStraightBullet";

	NpcBullet::Init();                       // 基底のセットアップ（モデル・親子付け等）
	BindParms();                             // 速度などをバインド
	if (globalParam_) globalParam_->SyncParamForGroup(groupName_);
}

