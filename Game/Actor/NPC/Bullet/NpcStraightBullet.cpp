#include "NpcStraightBullet.h"

#include "utility/ParameterEditor/GlobalParameter.h"

/// ===================================================
/// 初期化
/// ===================================================
void NpcStraightBullet::Init(){
	if (isInitialized_)return; //< 早期return

	// 使用モデル
	modelName_ = "cube.obj";

	//外部出力パラメータ保存名
	groupName_ = "NpcHomingBullet";

	// ブローバルパラメータとモデルの初期化
	NpcBullet::Init();
}

/// ===================================================
/// 更新
/// ===================================================
void NpcStraightBullet::Update(){
	if (!isActive_) return;

	//ホーミング対象がいたらホーミングいなかったらまっすぐ撃つように
	if (target_){}
	else{ NpcBullet::Update(); }
}

/// ===================================================
/// パラメータ同期
/// ===================================================
void NpcStraightBullet::BindParms(){ NpcBullet::BindParms(); } //< スピード

/// ===================================================
///  ホーミングtargetを設定
/// ===================================================
void NpcStraightBullet::SetTarget(const BaseObject* target){ target_ = target; }