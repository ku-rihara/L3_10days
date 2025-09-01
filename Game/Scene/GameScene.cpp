#include "GameScene.h"
#include "base/TextureManager.h"
// class
#include "3d/Object3DRegistry.h"
#include "utility/ParticleEditor/ParticleManager.h"

// math
#include "Frame/Frame.h"
#include "Lighrt/Light.h"
#include "Scene/Manager/SceneManager.h"

#include "Animation/AnimationRegistry.h"
#include "Pipeline/Object3DPiprline.h"
#include "ShadowMap/ShadowMap.h"

#include "Pipeline/BoundaryPipeline.h"

#include <imgui.h>

GameScene::GameScene() {}
GameScene::~GameScene() {}

void GameScene::Init() {

	BaseScene::Init();
	skuBox_ = std::make_unique<SkyBox>();
	skuBox_->Init();

	boundary_ = Boundary::GetInstance();
	boundary_->Init();

	ParticleManager::GetInstance()->SetViewProjection(&viewProjection_);
}

void GameScene::Update() {

	/// debugCamera
	debugCamera_->Update();
	Debug();

	boundary_->Update();

	/// objectの行列の更新をする
	Object3DRegistry::GetInstance()->UpdateAll();
	AnimationRegistry::GetInstance()->UpdateAll(Frame::DeltaTimeRate());


	// viewProjection 更新
	ViewProjectionUpdate();

	if (input_->TrrigerKey(DIK_RETURN)) {
		SceneManager::GetInstance()->ChangeScene("TITLE");
	}

	ParticleManager::GetInstance()->Update();
}

/// ===================================================
/// モデル描画
/// ===================================================
void GameScene::ModelDraw() {

	ID3D12GraphicsCommandList* commandList = DirectXCommon::GetInstance()->GetCommandList();
	/// 境界の描画
	BoundaryPipeline* boundaryPipeline = BoundaryPipeline::GetInstance();
	boundaryPipeline->PreDraw(commandList);
	boundaryPipeline->Draw(commandList, viewProjection_);

	/// オブジェクトの描画
	Object3DPiprline::GetInstance()->PreDraw(commandList);
	Object3DRegistry::GetInstance()->DrawAll(viewProjection_);
	ParticleManager::GetInstance()->Draw(viewProjection_);


}

/// ===================================================
/// SkyBox描画
/// ===================================================
void GameScene::SkyBoxDraw() {
	skuBox_->Draw(viewProjection_);
}

/// ======================================================
/// スプライト描画
/// ======================================================
void GameScene::SpriteDraw() {}

/// ======================================================
/// 影描画
/// ======================================================
void GameScene::DrawShadow() {
	Object3DRegistry::GetInstance()->DrawAllShadow(viewProjection_);
}

void GameScene::Debug() {
#ifdef _DEBUG

	ImGui::Begin("Object");

	ShadowMap::GetInstance()->DebugImGui();

	ImGui::End();

#endif
}

// ビュープロジェクション更新
void GameScene::ViewProjectionUpdate() {
	BaseScene::ViewProjectionUpdate();
}

void GameScene::ViewProssess() {
	viewProjection_.UpdateMatrix();
}
