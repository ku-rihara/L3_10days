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
#include "Pipeline/BoundaryEdgePipeline.h"

#include <imgui.h>

GameScene::GameScene() {}
GameScene::~GameScene() {}

void GameScene::Init() {

	BaseScene::Init();

	// 生成
	skuBox_ = std::make_unique<SkyBox>();
	player_ = std::make_unique<Player>();
    enemyStation_ = std::make_unique<EnemyStation>("EnemyStation");
	gameCamera_ = std::make_unique<GameCamera>();
    //====================================生成===================================================
    skuBox_ = std::make_unique<SkyBox>();
    player_ = std::make_unique<Player>();   
    gameCamera_ = std::make_unique<GameCamera>();
    testGround_ = std::make_unique<TestGround>();

	// 初期化
	enemyStation_->Init();
	player_->Init();
	gameCamera_->Init();
	skuBox_->Init();
   //====================================初期化===================================================
    skuBox_->Init();
    player_->Init();
    gameCamera_->Init();
    testGround_->Init();


	boundary_ = Boundary::GetInstance();
	boundary_->Init();

	// ParticleViewSet
	ParticleManager::GetInstance()->SetViewProjection(&viewProjection_);
   //====================================Class Set===================================================
    player_->SetViewProjection(&viewProjection_);
    gameCamera_->SetTarget(&player_->GetTransform());

    // ParticleViewSet
    ParticleManager::GetInstance()->SetViewProjection(&viewProjection_);
}

void GameScene::Update() {

	/// debugCamera
	debugCamera_->Update();
	Debug();

	// class Update
	gameCamera_->Update();
	player_->Update();
	enemyStation_->Update();
	boundary_->Update();
    // class Update
    gameCamera_->Update();
    player_->Update();
    skuBox_->Update();
    testGround_->Update();

	/// objectの行列の更新をする
	Object3DRegistry::GetInstance()->UpdateAll();
	AnimationRegistry::GetInstance()->UpdateAll(Frame::DeltaTimeRate());

	// viewProjection 更新
	ViewProjectionUpdate();

	// Scene Change
	if (input_->TrrigerKey(DIK_RETURN)) {
		SceneManager::GetInstance()->ChangeScene("TITLE");
	}

	// Particle AllUpdate
	ParticleManager::GetInstance()->Update();
}

/// ===================================================
/// モデル描画
/// ===================================================
void GameScene::ModelDraw() {

	ID3D12GraphicsCommandList* commandList = DirectXCommon::GetInstance()->GetCommandList();

	/// オブジェクトの描画
	Object3DPiprline::GetInstance()->PreDraw(commandList);
	Object3DRegistry::GetInstance()->DrawAll(viewProjection_);
	ParticleManager::GetInstance()->Draw(viewProjection_);


	/// 境界の描画
	BoundaryPipeline* boundaryPipeline = BoundaryPipeline::GetInstance();
	boundaryPipeline->PreDraw(commandList);
	boundaryPipeline->Draw(commandList, viewProjection_);

	/// 境界の穴の境界を描画
	BoundaryEdgePipeline* boundaryEdgePipeline = BoundaryEdgePipeline::GetInstance();
	boundaryEdgePipeline->PreDraw(commandList);
	boundaryEdgePipeline->Draw(commandList, viewProjection_);

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
	player_->AdjustParam();
	enemyStation_->ShowGui();
	ShadowMap::GetInstance()->DebugImGui();
	ImGui::End();
    ImGui::Begin("Object");
    player_->AdjustParam();
    gameCamera_->AdjustParam();
    ShadowMap::GetInstance()->DebugImGui();
    ImGui::End();

#endif
}

// ビュープロジェクション更新
void GameScene::ViewProjectionUpdate() {
	viewProjection_.matView_ = gameCamera_->GetViewProjection().matView_;
	viewProjection_.matProjection_ = gameCamera_->GetViewProjection().matProjection_;
	viewProjection_.cameraMatrix_ = gameCamera_->GetViewProjection().cameraMatrix_;
	viewProjection_.rotation_ = gameCamera_->GetViewProjection().rotation_;
	BaseScene::ViewProjectionUpdate();
}

void GameScene::ViewProssess() {
	viewProjection_.UpdateMatrix();
    BaseScene::ViewProjectionUpdate();
}

void GameScene::ViewProssess() {
    viewProjection_.matView_       = gameCamera_->GetViewProjection().matView_;
    viewProjection_.matProjection_ = gameCamera_->GetViewProjection().matProjection_;
    viewProjection_.cameraMatrix_  = gameCamera_->GetViewProjection().cameraMatrix_;
    viewProjection_.rotation_      = gameCamera_->GetViewProjection().rotation_;
    viewProjection_.TransferMatrix();
}
