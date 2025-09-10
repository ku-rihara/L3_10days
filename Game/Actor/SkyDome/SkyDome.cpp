#include "SkyDome.h"

/// engine
#include "3d/ModelManager.h"
#include "Frame/Frame.h"


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
	/// uv scale
	float uvScale = 4.0f;
	obj3d_->material_.materialData_->uvTransform.m[0][0] = uvScale;
	obj3d_->material_.materialData_->uvTransform.m[1][1] = uvScale;

	float scale = 1500.0f * 4.0f;
	obj3d_->transform_.scale_ = { scale, scale, scale };
	obj3d_->Update();

	time_ = 0.0f;
}

void SkyDome::Update() {}

void SkyDome::Draw(const ViewProjection& _vp) {
	obj3d_->Draw(_vp);
}


BackgroundObj::BackgroundObj() {
	const std::string instanceName = "skydome.obj";
	obj3d_ = std::make_unique<Object3d>();
	ModelManager::GetInstance()->LoadModel(instanceName);
	obj3d_->transform_.Init();
	obj3d_->SetModel(instanceName);
	obj3d_->CreateWVPResource();
	obj3d_->CreateShadowMap();
	obj3d_->CreateMaterialResource();
	obj3d_->SetTexture("SkyDome.png");
}

BackgroundObj::~BackgroundObj() = default;
