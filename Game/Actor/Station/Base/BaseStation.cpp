#include "BaseStation.h"

#include "imgui.h"

BaseStation::BaseStation(const std::string& name):
	name_(name){}

/// ===================================================
/// 初期化
/// ===================================================
void BaseStation::Init(){
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
	const std::string path = fileDirectory_ + "/" + name_;
	if (ImGui::TreeNode(name_.c_str())){
		//設定の保存
		if (ImGui::Button("save")){ SaveData(); }

		ImGui::SameLine();

		//設定のロード
		if (ImGui::Button("load")){ LoadData(); }

		//transform
		if (ImGui::CollapsingHeader("Transform")){
			ImGui::DragFloat3("Scale",&baseTransform_.scale_.x,0.01f);
			ImGui::DragFloat3("Rotate",&baseTransform_.rotation_.x,0.01f);
			ImGui::DragFloat3("Translate",&baseTransform_.translation_.x,0.01f);
		}

		//Adjustment items
		globalParam_->ParamLoadForImGui(name_,path);

		ImGui::TreePop();
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
	const std::string path = fileDirectory_ + "/" + name_;

	globalParam_->LoadFile(name_,path);
	globalParam_->SyncParamForGroup(name_);
}

/// ===================================================
/// パラメータ保存
/// ===================================================
void BaseStation::SaveData(){
	const std::string path = fileDirectory_ + "/" + name_;
	globalParam_->SaveFile(name_,path);
}
