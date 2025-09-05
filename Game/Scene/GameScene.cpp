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
#include "Actor/NPC/BoundaryBreaker/Installer/BoundaryBreakerInstaller.h"

#include "Pipeline/BoundaryPipeline.h"
#include "Pipeline/BoundaryEdgePipeline.h"
#include "Pipeline/BoundaryShardPipeline.h"
#include "Pipeline/MiniMapIconPipeline.h"
#include "Pipeline/MiniMapPipeline.h"
#include "Pipeline/EffectPipelines/PlayerOutsideWarningPipeline.h"

#include <imgui.h>

GameScene::GameScene(){}
GameScene::~GameScene(){}

void GameScene::Init(){
	BaseScene::Init();
	// 生成
	//====================================生成===================================================
	skyDome_ = std::make_unique<SkyDome>();
	player_ = std::make_unique<Player>();
	stations_[FactionType::Ally] = std::make_unique<PlayerStation>("PlayerStation");
	stations_[FactionType::Enemy] = std::make_unique<EnemyStation>("EnemyStation");
	gameCamera_ = std::make_unique<GameCamera>();


	UnitDirectorConfig cfg;
	cfg.squadSize = 4; // 攻撃小隊の目安
	cfg.preferSticky = true; // 既存ロール優先で揺れを減らす
	cfg.defendHoldRadius = 8.0f; // この距離以内なら防衛はその場オービット
	director_ = std::make_unique<QuotaUnitDirector>(cfg);

	/// UI -----
	miniMap_ = std::make_unique<MiniMap>();
	uis_ = std::make_unique<GameUIs>();

	/// Effect -----
	outsideWarning_ = std::make_unique<PlayerOutsideWarning>();


	//====================================初期化===================================================
	skyDome_->Init();
	player_->Init();
	Installer::InstallStations(stations_[FactionType::Ally].get(),
							   stations_[FactionType::Enemy].get(),
							   director_.get());

	const Vector3 enemyStaitonPos = stations_[FactionType::Enemy]->GetWorldPosition();
	Installer::InstallBoundaryBreaker(boundaryBreakers_,enemyStaitonPos,2,
									  stations_[FactionType::Ally].get());

	gameCamera_->Init();
	//testGround_->Init();

	boundary_ = Boundary::GetInstance();
	boundary_->Init();

	/// UI -----
	miniMap_->Init(stations_[FactionType::Ally].get(),stations_[FactionType::Enemy].get());
	miniMap_->RegisterPlayer(player_.get());
	uis_->Init();

	/// Effect -----
	outsideWarning_->Init();

	// ParticleViewSet
	ParticleManager::GetInstance()->SetViewProjection(&viewProjection_);


	//====================================Class Set===================================================
	player_->SetViewProjection(&viewProjection_);
	gameCamera_->SetTarget(&player_->GetTransform());
	gameCamera_->SetPlayer(player_.get());

	// ParticleViewSet
	ParticleManager::GetInstance()->SetViewProjection(&viewProjection_);
}

void GameScene::Update(){
	/// debugCamera
	debugCamera_->Update();
	Debug();

	// class Update
	boundary_->Update();
	player_->Update();
	gameCamera_->Update();
	for (auto& kv : stations_){ kv.second->Update(); }
	for (auto& bb : boundaryBreakers_)bb->Update();
	skyDome_->Update();

	miniMap_->Update();
	uis_->Update(player_.get());

	/// objectの行列の更新をする
	Object3DRegistry::GetInstance()->UpdateAll();
	AnimationRegistry::GetInstance()->UpdateAll(Frame::DeltaTimeRate());

	// viewProjection 更新
	ViewProjectionUpdate();

	// Scene Change
	if (input_->TrrigerKey(DIK_RETURN)){ SceneManager::GetInstance()->ChangeScene("TITLE"); }

	// Particle AllUpdate
	ParticleManager::GetInstance()->Update();
}

/// ===================================================
/// モデル描画
/// ===================================================
void GameScene::ModelDraw(){
	ID3D12GraphicsCommandList* commandList = DirectXCommon::GetInstance()->GetCommandList();

	/// 天球を描画
	Object3DPiprline::GetInstance()->PreDraw(commandList);
	skyDome_->Draw(viewProjection_);

	/// 境界の描画
	BoundaryPipeline* boundaryPipeline = BoundaryPipeline::GetInstance();
	boundaryPipeline->PreDraw(commandList);
	boundaryPipeline->Draw(commandList,viewProjection_);

	/// オブジェクトの描画
	Object3DPiprline::GetInstance()->PreDraw(commandList);
	Object3DRegistry::GetInstance()->DrawAll(viewProjection_);
	ParticleManager::GetInstance()->Draw(viewProjection_);


	/// 境界の破片の描画
	BoundaryShardPipeline* boundaryShardPipeline = BoundaryShardPipeline::GetInstance();
	boundaryShardPipeline->PreDraw(commandList);
	boundaryShardPipeline->Draw(commandList,viewProjection_);

	/// 境界の穴の境界を描画
	BoundaryEdgePipeline* boundaryEdgePipeline = BoundaryEdgePipeline::GetInstance();
	boundaryEdgePipeline->PreDraw(commandList);
	boundaryEdgePipeline->Draw(commandList,viewProjection_);

	MiniMapPipeline* miniMapPipeline = MiniMapPipeline::GetInstance();
	miniMapPipeline->PreDraw(commandList);
	miniMapPipeline->Draw(commandList,miniMap_.get());
}

/// ===================================================
/// SkyBox描画
/// ===================================================
void GameScene::SkyBoxDraw(){}

/// ======================================================
/// スプライト描画
/// ======================================================
void GameScene::SpriteDraw(){
	uis_->Draw();

	/// ミニマップ描画
	miniMap_->DrawMiniMap();

	ID3D12GraphicsCommandList* commandList = DirectXCommon::GetInstance()->GetCommandList();
	/// UI用に
	MiniMapIconPipeline* miniMapIconPipeline = MiniMapIconPipeline::GetInstance();
	miniMapIconPipeline->PreDraw(commandList);
	miniMapIconPipeline->Draw(commandList,miniMap_.get());

	PlayerOutsideWarningPipeline* outsideWarning = PlayerOutsideWarningPipeline::GetInstance();
	outsideWarning->PreDraw(commandList);
	outsideWarning->Draw(commandList,outsideWarning_.get());
}

/// ======================================================
/// 影描画
/// ======================================================
void GameScene::DrawShadow(){
	//Object3DRegistry::GetInstance()->DrawAllShadow(viewProjection_);
}

void GameScene::Debug(){
#ifdef _DEBUG

	ImGui::Begin("Object");
	player_->AdjustParam();
	for (auto& kv : stations_){ kv.second->ShowGui(); }
	gameCamera_->AdjustParam();
	ShadowMap::GetInstance()->DebugImGui();
	ImGui::End();

#endif
}

// ビュープロジェクション更新
void GameScene::ViewProjectionUpdate(){ BaseScene::ViewProjectionUpdate(); }

void GameScene::ViewProssess(){
	viewProjection_.matView_ = gameCamera_->GetViewProjection().matView_;
	viewProjection_.matProjection_ = gameCamera_->GetViewProjection().matProjection_;
	viewProjection_.cameraMatrix_ = gameCamera_->GetViewProjection().cameraMatrix_;
	viewProjection_.rotation_ = gameCamera_->GetViewProjection().rotation_;
	viewProjection_.TransferMatrix();
}