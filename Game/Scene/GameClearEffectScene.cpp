#include "GameClearEffectScene.h"

/// externals
#include <imgui.h>

/// engine
#include "Pipeline/Object3DPiprline.h"
#include "3d/Object3DRegistry.h"
#include "utility/ParticleEditor/ParticleManager.h"

/// game
#include "Object/SceneObject.h"

GameClearEffectScene::GameClearEffectScene() = default;
GameClearEffectScene::~GameClearEffectScene() = default;


void GameClearEffectScene::Init() {
	BaseScene::Init();

	using UObj = std::unique_ptr<SceneObject>;

	/// camera setting
	viewProjection_.translation_ = { -85.700f, 43.000f, 103.200f };
	viewProjection_.rotation_ = { 0.153f, 2.374f, 0.000f };

	/// instance create
	UObj station = std::make_unique<SceneObject>("EnemyBase.obj");
	station->SetScale(Vector3(1, 1, 1) * 10.0f);
	sceneObj_.emplace_back(std::move(station));

	/// 天球
	UObj skydome = std::make_unique<SceneObject>("skydome.obj");
	skydome->SetScale({ 1500.0f * 3.0f, 1500.0f * 3.0f, 1500.0f * 3.0f });
	sceneObj_.emplace_back(std::move(skydome));

	UObj bg = std::make_unique<SceneObject>("skydome.obj");
	bg->SetScale({ 1500.0f * 4.0f, 1500.0f * 4.0f, 1500.0f * 4.0f });
	bg->GetObj3d()->SetTexture("SkyDome.png");
	sceneObj_.emplace_back(std::move(bg));


	ParticleManager::GetInstance()->SetViewProjection(&viewProjection_);
}

void GameClearEffectScene::Update() {
	Debug();
	ViewProjectionUpdate();

	for (auto& obj : sceneObj_) {
		obj->Update();
	}

	/// シーン共通の更新
	Object3DRegistry::GetInstance()->UpdateAll();
	ParticleManager::GetInstance()->Update();
}

void GameClearEffectScene::ModelDraw() {
	/// commandList取得
	ID3D12GraphicsCommandList* commandList = DirectXCommon::GetInstance()->GetCommandList();
	Object3DPiprline::GetInstance()->PreDraw(commandList);
	Object3DRegistry::GetInstance()->DrawAll(viewProjection_);
	ParticleManager::GetInstance()->Draw(viewProjection_);
}

void GameClearEffectScene::SpriteDraw() {}

void GameClearEffectScene::SkyBoxDraw() {}

void GameClearEffectScene::DrawShadow() {}

void GameClearEffectScene::Debug() {
#ifdef _DEBUG
	ImGui::Begin("Camera");
	ImGui::DragFloat3("pos", &viewProjection_.translation_.x, 0.1f);
	ImGui::DragFloat3("rotate", &viewProjection_.rotation_.x, 0.1f);
	ImGui::End();
#endif
}

void GameClearEffectScene::ViewProssess() {
	viewProjection_.UpdateMatrix();
}


