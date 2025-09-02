#include "BaseStation.h"

#include "Actor/NPC/NPC.h"

#include <algorithm>
#include "imgui.h"

BaseStation::BaseStation(const std::string& name):
	name_(name){}

BaseStation::~BaseStation() = default;

/// ===================================================
/// 初期化
/// ===================================================
void BaseStation::Init(){
	if (isInitialized_) { return; }
	isInitialized_ = true;

	obj3d_.reset(Object3d::CreateModel("cube.obj"));
	BaseObject::Init();
	obj3d_->transform_.parent_ = &baseTransform_;

	globalParam_ = GlobalParameter::GetInstance();

	//メイングループの設定
	globalParam_->CreateGroup(name_,true);

	//重複バインドを防ぐ
	globalParam_->ClearBindingsForGroup(name_);
	BindParms();

	//パラメータ同期
	globalParam_->SyncParamForGroup(name_);

	LoadData();
	// 初期座標適用
	baseTransform_.translation_ = initialPosition_;
}

/// ===================================================
/// 更新
/// ===================================================
void BaseStation::Update(){ BaseObject::Update(); }

/// ===================================================
/// gui表示
/// ===================================================
void BaseStation::ShowGui(){
	const std::string path = fileDirectory_ ;
	if (ImGui::CollapsingHeader(name_.c_str())) {
		ImGui::PushID(name_.c_str());
		
		globalParam_->ParamLoadForImGui(name_, path);
		
		ImGui::SameLine();

		globalParam_->ParamSaveForImGui(name_, path);

		//transform
		if (ImGui::TreeNode("Transform")){
			ImGui::DragFloat3("Scale",&baseTransform_.scale_.x,0.01f);
			ImGui::DragFloat3("Rotate",&baseTransform_.rotation_.x,0.01f);
			if (ImGui::DragFloat3("Translate", &baseTransform_.translation_.x, 0.01f)) {initialPosition_ = baseTransform_.GetWorldPos();};
			ImGui::TreePop();
		}



		ImGui::PopID();
	}
}

/// ===================================================
/// パラメータの適用
/// ===================================================
void BaseStation::BindParms(){ globalParam_->Bind(name_,"initialPosition",&initialPosition_); }

/// ===================================================
/// パラメータの読み込み
/// ===================================================
void BaseStation::LoadData(){
	const std::string path = fileDirectory_ ;

	globalParam_->LoadFile(name_,path);
	globalParam_->SyncParamForGroup(name_);
}

/// ===================================================
/// パラメータ保存
/// ===================================================
void BaseStation::SaveData(){
	const std::string path = fileDirectory_;
	globalParam_->SaveFile(name_,path);
}

/////////////////////////////////////////////////////////////////////////////////////////
//		accessor
/////////////////////////////////////////////////////////////////////////////////////////

void BaseStation::SetRivalStation(BaseStation* rival) { pRivalStation_ = rival; }

BaseStation* BaseStation::GetRivalStation() const {return pRivalStation_;}

/// ===================================================
/// 派閥を設定
/// ===================================================
void BaseStation::SetFaction(FactionType type) {faction_ = type;}

FactionType BaseStation::GetFactionType() const { return faction_; }

/// ===================================================
/// リストを掃除
/// ===================================================
void BaseStation::CleanupSpawnedList() {
	spawned_.erase(
		std::remove_if(spawned_.begin(), spawned_.end(),
					   [](const NpcHandle& npc) {
		return !npc || !npc->GetIsAlive();
	}),
		spawned_.end()
	);
}