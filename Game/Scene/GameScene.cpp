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
#include "Actor/Station/Enemy/EnemyStation.h"
#include "Actor/Station/Player/PlayerStation.h"
#include "Actor/Station/Installer/StationsInstaller.h"

#include "Pipeline/BoundaryPipeline.h"
#include "Pipeline/BoundaryEdgePipeline.h"
#include "Pipeline/BoundaryShardPipeline.h"
#include "Pipeline/MiniMapIconPipeline.h"

#include <imgui.h>

GameScene::GameScene() {}
GameScene::~GameScene() {}

void GameScene::Init() {

	BaseScene::Init();

	// 生成
	//====================================生成===================================================
	skuBox_ = std::make_unique<SkyBox>();
	player_ = std::make_unique<Player>();
	stations_[FactionType::Ally] = std::make_unique<PlayerStation>("PlayerStation");
	stations_[FactionType::Enemy] = std::make_unique<EnemyStation>("EnemyStation");
	gameCamera_ = std::make_unique<GameCamera>();
	//testGround_ = std::make_unique<TestGround>();

	/// UI -----
	miniMap_ = std::make_unique<MiniMap>();


	//====================================初期化===================================================
	skuBox_->Init();
	player_->Init();
	Installer::InstallStations(stations_[FactionType::Ally], stations_[FactionType::Enemy]);
	gameCamera_->Init();
	//testGround_->Init();

	boundary_ = Boundary::GetInstance();
	boundary_->Init();

	/// UI -----
	miniMap_->Init(stations_[FactionType::Ally].get(), stations_[FactionType::Enemy].get());
	miniMap_->RegisterPlayer(player_.get());

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
	boundary_->Update();
	player_->Update();
	gameCamera_->Update();
	for (auto& kv : stations_) { kv.second->Update(); }
	skuBox_->Update();
	//testGround_->Update();

	miniMap_->Update();

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


	/// 境界の破片の描画
	BoundaryShardPipeline* boundaryShardPipeline = BoundaryShardPipeline::GetInstance();
	boundaryShardPipeline->PreDraw(commandList);
	boundaryShardPipeline->Draw(commandList, viewProjection_);

	/// 境界の描画
	BoundaryPipeline* boundaryPipeline = BoundaryPipeline::GetInstance();
	boundaryPipeline->PreDraw(commandList);
	boundaryPipeline->Draw(commandList, viewProjection_);

	/// 境界の穴の境界を描画
	BoundaryEdgePipeline* boundaryEdgePipeline = BoundaryEdgePipeline::GetInstance();
	boundaryEdgePipeline->PreDraw(commandList);
	boundaryEdgePipeline->Draw(commandList, viewProjection_);


	/// UI用に
	MiniMapIconPipeline* miniMapIconPipeline = MiniMapIconPipeline::GetInstance();
	miniMapIconPipeline->PreDraw(commandList);
	miniMapIconPipeline->Draw(commandList, miniMap_.get());
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
void GameScene::SpriteDraw() {
	miniMap_->DrawMiniMap();

	ID3D12GraphicsCommandList* commandList = DirectXCommon::GetInstance()->GetCommandList();
	/// UI用に
	MiniMapIconPipeline* miniMapIconPipeline = MiniMapIconPipeline::GetInstance();
	miniMapIconPipeline->PreDraw(commandList);
	miniMapIconPipeline->Draw(commandList, miniMap_.get());
}

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
	for (auto& kv : stations_) { kv.second->ShowGui(); }
	gameCamera_->AdjustParam();
	ShadowMap::GetInstance()->DebugImGui();
	ImGui::End();

#endif
}

// ビュープロジェクション更新
void GameScene::ViewProjectionUpdate() {
	BaseScene::ViewProjectionUpdate();
}

void GameScene::ViewProssess() {
	viewProjection_.matView_ = gameCamera_->GetViewProjection().matView_;
	viewProjection_.matProjection_ = gameCamera_->GetViewProjection().matProjection_;
	viewProjection_.cameraMatrix_ = gameCamera_->GetViewProjection().cameraMatrix_;
	viewProjection_.rotation_ = gameCamera_->GetViewProjection().rotation_;
	viewProjection_.TransferMatrix();
}
