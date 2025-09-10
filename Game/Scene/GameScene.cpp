#include "GameScene.h"
#include "base/TextureManager.h"
// class
#include "3d/Object3DRegistry.h"
#include "utility/ParticleEditor/ParticleManager.h"
#include "Collider/CollisionManager.h"

// math
#include "Frame/Frame.h"
#include "Lighrt/Light.h"
#include "Scene/Manager/SceneManager.h"

#include "Actor/NPC/BoundaryBreaker/Installer/BoundaryBreakerInstaller.h"
#include "Actor/Station/Enemy/EnemyStation.h"
#include "Actor/Station/Installer/StationsInstaller.h"
#include "Actor/Station/Player/PlayerStation.h"
#include "Actor/Spline/Spline.h"
#include "Actor/NPC/EnemyNPC.h"
#include "Actor/GameController/GameController.h"
#include "Actor/GameController/GameScore.h"
#include "Actor/ExpEmitter/ExpEmitter.h"

#include "Animation/AnimationRegistry.h"
#include "ShadowMap/ShadowMap.h"
#include "Pipeline/Object3DPiprline.h"
#include "Pipeline/BoundaryEdgePipeline.h"
#include "Pipeline/BoundaryPipeline.h"
#include "Pipeline/BoundaryShardPipeline.h"
#include "Pipeline/EffectPipelines/GameScreenEffectPipeline.h"
#include "Pipeline/EffectPipelines/PlayerOutOfFieldWarningEffectPipeline.h"
#include "Pipeline/Line3DPipeline.h"
#include "Pipeline/MiniMapIconPipeline.h"
#include "Pipeline/MiniMapPipeline.h"

/// effects
#include "Actor/Effects/PlayerEngineEffect/PlayerEngineEffect.h"
#include "Actor/Effects/PlayerLocus/PlayerLocusEffect.h"

/// option
#include "Option/GameOption.h"

#include <imgui.h>
#include <vector>

GameScene::GameScene() = default;
GameScene::~GameScene() = default;

void GameScene::Init() {
	BaseScene::Init();

	// option load
	GameOption::GetInstance()->Init();

	gameController_ = std::make_unique<GameController>();
	GameScore::GetInstance()->ScoreReset();

	// 爆発エミッター
	ExpEmitter::GetInstance()->Init();

	// 生成
	//====================================生成===================================================
	skyDome_ = std::make_unique<SkyDome>();
	player_ = std::make_unique<Player>();
	stations_[FactionType::Ally] = std::make_unique<PlayerStation>("PlayerStation");
	stations_[FactionType::Enemy] = std::make_unique<EnemyStation>("EnemyStation");
	gameCamera_ = std::make_unique<GameCamera>();
	lockOn_ = std::make_unique<LockOn>();
	//std::array<std::unique_ptr<ParticleEmitter>, 5> expEmitter_;

	UnitDirectorConfig cfg;
	cfg.squadSize = 4; // 攻撃小隊の目安
	cfg.preferSticky = true; // 既存ロール優先で揺れを減らす
	cfg.defendHoldRadius = 8.0f; // この距離以内なら防衛はその場オービット
	director_ = std::make_unique<QuotaUnitDirector>(cfg);
	routesCollection_ = std::make_unique<RouteCollection>();

	/// UI -----
	miniMap_ = std::make_unique<MiniMap>();
	uis_ = std::make_unique<GameUIs>();

	/// Effect -----
	outsideWarning_ = std::make_unique<GameScreenEffect>();
	engineEffect_ = std::make_unique<PlayerEngineEffect>();
	playerLocusEffect_ = std::make_unique<PlayerLocusEffect>();


	//====================================初期化===================================================
	skyDome_->Init();
	player_->Init();
	lockOn_->Init();
	routesCollection_->Init();

	//ルートを渡す
	for (auto& station : stations_) { station.second->SetRouteCollection(routesCollection_.get()); }

	Installer::InstallStations(stations_[FactionType::Ally].get(),
		stations_[FactionType::Enemy].get(),
		director_.get());
	//ステーションごとのルート
	stations_[FactionType::Ally]->SetRoute(routesCollection_->GetRoute(RouteType::AllyDifence));
	stations_[FactionType::Enemy]->SetRoute(routesCollection_->GetRoute(RouteType::EnemyDirence));


	//プレイヤーを攻撃対象に追加
	auto enemyStation = dynamic_cast<EnemyStation*>(stations_[FactionType::Enemy].get());
	enemyStation->SetPlayerPtr(player_.get());

	const Vector3 enemyStaitonPos = stations_[FactionType::Enemy]->GetWorldPosition();
	Installer::InstallBoundaryBreakers(boundaryBreakers_,
		stations_[FactionType::Ally].get(),
		stations_[FactionType::Enemy].get(),
		2);

	gameCamera_->Init();

	boundary_ = Boundary::GetInstance();
	boundary_->Init();
	boundary_->ResetHoles();

	/// UI -----
	miniMap_->Init(stations_[FactionType::Ally].get(), stations_[FactionType::Enemy].get());
	miniMap_->RegisterPlayer(player_.get());
	uis_->Init();

	/// Effect -----
	outsideWarning_->Init();
	engineEffect_->Init();
	engineEffect_->SetPlayer(player_.get());
	playerLocusEffect_->Init(player_.get());


	//====================================Class Set===================================================
	player_->SetViewProjection(&viewProjection_);
	player_->SetGameCamera(gameCamera_.get());
	player_->SetLockOn(lockOn_.get());
	gameCamera_->SetTarget(&player_->GetTransform());
	gameCamera_->SetPlayer(player_.get());

	gameController_->SetPlayer(player_.get());
	gameController_->SetEnemyStation(stations_[FactionType::Enemy].get());
	gameController_->SetPlayerStation(stations_[FactionType::Ally].get());

	// ParticleViewSet
	ParticleManager::GetInstance()->SetViewProjection(&viewProjection_);

	/// ====================================
	/// pause init
	/// ====================================

	pause_ = std::make_unique<Pause>();
	pause_->Init();


	/// BGMの再生
	bgmId_ = audio_->LoadWave("./resources/Sound/BGM/InGameBGM.wav");
	audio_->PlayBGM(bgmId_, 0.05f);
}

void GameScene::Update() {
	/// ポーズの更新
	PauseUpdate();
	if (!pause_->IsPause()) {
		/// ゲームの更新
		GameUpdate();
	}

	/// ゲームの状態チェック
	/// TODO: 各演出が終了してから遷移する
	if (gameController_->GetIsGameClear()) {
		audio_->StopBGM(bgmId_);
		SceneManager::GetInstance()->ChangeScene("GAMECLEAREFFECT");
		return;
	} else if (gameController_->GetIsGameOver()) {
		audio_->StopBGM(bgmId_);
		SceneManager::GetInstance()->ChangeScene("GAMEOVEREFFECT");
		return;
	}


	/// ポーズからタイトルに戻る
	if (pause_->IsSceneChange()) {
		audio_->StopBGM(bgmId_);
		SceneManager::GetInstance()->ChangeScene("TITLE");
		return;
	}


#ifdef _DEBUG /// Scene Change (Debug)
	// Scene Change
	if (input_->TrrigerKey(DIK_RETURN)) {
		audio_->StopBGM(bgmId_);
		SceneManager::GetInstance()->ChangeScene("TITLE");
		return;
	}

	/// debug command
	if (input_->TrrigerKey(DIK_0)) { player_->SetHP(0.0f); }
#endif /// Scene Change (Debug)
}

/// ===================================================
/// モデル描画
/// ===================================================
void GameScene::ModelDraw() {
	GameModelDraw();

	if (pause_->IsPause()) { PauseModelDraw(); }
}

/// ===================================================
/// SkyBox描画
/// ===================================================
void GameScene::SkyBoxDraw() {}

/// ======================================================
/// スプライト描画
/// ======================================================
void GameScene::SpriteDraw() {
	GameSpriteDraw();
	if (pause_->IsPause()) { PauseSpriteDraw(); }
}

/// ======================================================
/// 影描画
/// ======================================================
void GameScene::DrawShadow() {
	// Object3DRegistry::GetInstance()->DrawAllShadow(viewProjection_);
}

void GameScene::Debug() {
#ifdef _DEBUG

	ImGui::Begin("Object");

	if (ImGui::Button("emit exp")) {
		ExpEmitter::GetInstance()->Emit({});
	}


	player_->AdjustParam();
	for (auto& kv : stations_) { kv.second->ShowGui(); }
	gameCamera_->AdjustParam();
	lockOn_->AdjustParam();
	ShadowMap::GetInstance()->DebugImGui();
	ImGui::End();

#endif
}

// ビュープロジェクション更新
void GameScene::ViewProjectionUpdate() { BaseScene::ViewProjectionUpdate(); }

void GameScene::ViewProssess() {
	viewProjection_.matView_ = gameCamera_->GetViewProjection().matView_;
	viewProjection_.matProjection_ = gameCamera_->GetViewProjection().matProjection_;
	viewProjection_.cameraMatrix_ = gameCamera_->GetViewProjection().cameraMatrix_;
	viewProjection_.rotation_ = gameCamera_->GetViewProjection().rotation_;
	viewProjection_.TransferMatrix();
}

void GameScene::GameUpdate() {
	Debug();

	// class Update
	boundary_->Update();
	player_->Update();
	gameCamera_->Update();

	for (auto& kv : stations_) {
		kv.second->Update();
	}
	for (auto& bb : boundaryBreakers_) {
		bb->Update();
	}
	skyDome_->Update();

	//----- それぞれのLockOn対象を取得 -----
	// EnemyNPCs
	std::vector<LockOn::LockOnVariant> targets;
	auto enemyStations = static_cast<EnemyStation*>(stations_[FactionType::Enemy].get());
	auto enemyNPCs = enemyStations->GetLiveNpcs();
	for (auto* npc : enemyNPCs) { targets.emplace_back(static_cast<EnemyNPC*>(npc)); }
	// boundaryBreakers
	for (auto& bb : boundaryBreakers_) { if (bb /*&&生きてたら*/) { targets.emplace_back(bb.get()); } }
	// baseStatuon
	for (auto& station : stations_) {
		if (station.second->GetHp() > 0/*&&生きてたら*/) {
			auto enemyStation = dynamic_cast<EnemyStation*>(stations_[FactionType::Enemy].get());
			targets.emplace_back(enemyStation);
		}
	}

	// lockOn更新
	lockOn_->Update(targets, player_.get(), viewProjection_, FactionType::Enemy);

	/// ui update
	miniMap_->Update();
	uis_->Update(player_.get());

	/// effect update
	playerLocusEffect_->Update();
	gameController_->Update();
	ExpEmitter::GetInstance()->Update();

	/// objectの行列の更新をする
	Object3DRegistry::GetInstance()->UpdateAll();
	AnimationRegistry::GetInstance()->UpdateAll(Frame::DeltaTimeRate());

	// viewProjection 更新
	ViewProjectionUpdate();

	// Particle AllUpdate
	ParticleManager::GetInstance()->Update();
}

void GameScene::PauseUpdate() {
	pause_->Update();
	GameOption* option = GameOption::GetInstance();
	option->Update();
	if ((option->GetIsDirtyThisFrame()
		|| option->GetPrevIsDirtyThisFrame())
		&& !option->GetIsOpen()) {
		player_->ClosedPaused();
	}
}

void GameScene::GameModelDraw() {
	ID3D12GraphicsCommandList* commandList = DirectXCommon::GetInstance()->GetCommandList();

	Line3DPipeline* line3dPipeline = Line3DPipeline::GetInstance();
	line3dPipeline->PreDraw(commandList);

	for (auto& stations : stations_) {
		stations.second->DrawDebug(viewProjection_);
	}

	/// 天球を描画
	Object3DPiprline::GetInstance()->PreDraw(commandList);
	skyDome_->DrawBG(viewProjection_);
	skyDome_->DrawSkyDome(viewProjection_);

	/// 境界の描画
	BoundaryPipeline* boundaryPipeline = BoundaryPipeline::GetInstance();
	boundaryPipeline->PreDraw(commandList);
	boundaryPipeline->Draw(commandList, viewProjection_);


	/// オブジェクトの描画
	Object3DPiprline::GetInstance()->PreDraw(commandList);
	Object3DRegistry::GetInstance()->DrawAll(viewProjection_);
	ParticleManager::GetInstance()->Draw(viewProjection_);
	//CollisionManager::GetInstance()->Draw(viewProjection_);

	/// 境界の破片の描画
	BoundaryShardPipeline* boundaryShardPipeline = BoundaryShardPipeline::GetInstance();
	boundaryShardPipeline->PreDraw(commandList);
	boundaryShardPipeline->Draw(commandList, viewProjection_);

	/// 境界の穴の境界を描画
	BoundaryEdgePipeline* boundaryEdgePipeline = BoundaryEdgePipeline::GetInstance();
	boundaryEdgePipeline->PreDraw(commandList);
	boundaryEdgePipeline->Draw(commandList, viewProjection_);

	MiniMapPipeline* miniMapPipeline = MiniMapPipeline::GetInstance();
	miniMapPipeline->PreDraw(commandList);
	miniMapPipeline->Draw(commandList, miniMap_.get());

	//旋回ルートの描画
	routesCollection_->DebugDraw(viewProjection_);
}

void GameScene::GameSpriteDraw() {
	ID3D12GraphicsCommandList* commandList = DirectXCommon::GetInstance()->GetCommandList();

	/// random noise + vignette
	GameScreenEffectPipeline* outsideWarning = GameScreenEffectPipeline::GetInstance();
	outsideWarning->PreDraw(commandList);
	outsideWarning->Draw(commandList, outsideWarning_.get());

	PlayerOutOfFieldWarningEffectPipeline* playerOutOfFieldWarning =
		PlayerOutOfFieldWarningEffectPipeline::GetInstance();
	playerOutOfFieldWarning->PreDraw(commandList);
	playerOutOfFieldWarning->Draw(commandList, gameController_.get());

	Sprite::PreDraw(commandList);
	gameController_->DrawOutOfFieldWarningTime();
	gameController_->DrawGameTimer();
	uis_->Draw();
	player_->ReticleDraw();
	lockOn_->Draw();
	player_->UIDraw();
	/// ミニマップ描画
	miniMap_->DrawMiniMapFrame();

	/// UI用に
	MiniMapIconPipeline* miniMapIconPipeline = MiniMapIconPipeline::GetInstance();
	miniMapIconPipeline->PreDraw(commandList);
	miniMapIconPipeline->Draw(commandList, miniMap_.get());

	Sprite::PreDraw(commandList);
	miniMap_->DrawMiniMapPlayerIcon();
}

void GameScene::PauseModelDraw() {}

void GameScene::PauseSpriteDraw() {
	ID3D12GraphicsCommandList* commandList = DirectXCommon::GetInstance()->GetCommandList();

	Sprite::PreDraw(commandList);
	pause_->Draw();

	GameOption::GetInstance()->Draw();
}