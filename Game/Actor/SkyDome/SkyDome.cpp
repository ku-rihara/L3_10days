#include "SkyDome.h"

/// engine
#include "3d/ModelManager.h"

void SkyDome::Init() {
	const std::string instanceName = "skydome.obj";

	// 新しいModelインスタンスを作成
	obj3d_ = std::make_unique<Object3d>();
	ModelManager::GetInstance()->LoadModel(instanceName);
	obj3d_->transform_.Init();
	obj3d_->SetModel(instanceName);
	obj3d_->CreateWVPResource();
	obj3d_->CreateShadowMap();
	obj3d_->CreateMaterialResource();

	obj3d_->material_.materialData_->enableLighting = false;
	float scale = 1500.0f;
	obj3d_->transform_.scale_ = { scale, scale, scale };
}

void SkyDome::Update() {

}
