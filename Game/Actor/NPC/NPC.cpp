#include "NPC.h"

#include "Actor/Station/Base/BaseStation.h"
#include "Frame/Frame.h"

/// ===================================================
/// 初期化
/// ===================================================
void NPC::Init() {
	obj3d_.reset(Object3d::CreateModel("cube.obj"));
	BaseObject::Init();
	obj3d_->transform_.parent_ = &baseTransform_;

	globalParam_ = GlobalParameter::GetInstance();

	// グループ名が入っていなければデフォルト
	if (groupName_.empty()) { groupName_ = "UnnamedNPC"; }

	//メイングループの設定
	globalParam_->CreateGroup(groupName_, true);

	//重複バインドを防ぐ
	globalParam_->ClearBindingsForGroup(groupName_);
	BindParms();

	//パラメータ同期
	globalParam_->SyncParamForGroup(groupName_);

	//LoadData();
}

/// ===================================================
/// 移動
/// ===================================================
void NPC::Move() {
	//ターゲットがなければ何もしない
	if (!target_)return;

	Vector3 dir = (target_->GetWorldPosition() - GetWorldPosition()).Normalize();

	float dt = Frame::DeltaTime();

	//移動
	baseTransform_.translation_ += dir * speed_ * dt;
}

/// ===================================================
/// 更新
/// ===================================================
void NPC::Update() {
	Move();
	BaseObject::Update();
}

/////////////////////////////////////////////////////////////////////////////////////////
//		パラメータ
/////////////////////////////////////////////////////////////////////////////////////////

/// ===================================================
/// パラメータの同期
/// ===================================================
void NPC::BindParms() { globalParam_->Bind(groupName_, "maxHP", &maxHP_); }

/// ===================================================
/// データ読み込み
/// ===================================================
void NPC::LoadData() {
	globalParam_->LoadFile(groupName_, fileDirectory_);
}

/// ===================================================
/// データ保存
/// ===================================================
void NPC::SaveData() { globalParam_->SaveFile(groupName_, fileDirectory_); }


/////////////////////////////////////////////////////////////////////////////////////////
//		accessor
/////////////////////////////////////////////////////////////////////////////////////////

/// ===================================================
///  ターゲットを設定
/// ===================================================
void NPC::SetTarget(const BaseStation* target) { target_ = target; }

/// ===================================================
///  派閥を設定
/// ===================================================
void NPC::SetFaction(FactionType faction) { faction_ = faction; }


