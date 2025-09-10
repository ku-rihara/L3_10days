/// scene
#include "TutorialScene.h"
#include "GameScene.h"
#include "Manager/SceneManager.h"

#include "base/TextureManager.h"
// class
#include "3d/Object3DRegistry.h"
#include "Pipeline/Object3DPiprline.h"
#include "utility/ParticleEditor/ParticleManager.h"
#include"Actor/ExpEmitter/ExpEmitter.h"

// math
#include "Frame/Frame.h"
#include <imgui.h>

/// game
#include "Option/GameOption.h"
#include "TitleActor/FighterAircraft/FighterAircraft.h"
#include <Actor/Station/Enemy/EnemyStation.h>
#include <Actor/Station/Player/PlayerStation.h>
#include <Pipeline/BoundaryEdgePipeline.h>
#include <Pipeline/BoundaryPipeline.h>
#include <Pipeline/BoundaryShardPipeline.h>
#include <Pipeline/Line3DPipeline.h>
#include <Pipeline/MiniMapPipeline.h>

/// effects
#include "Actor/Effects/PlayerEngineEffect/PlayerEngineEffect.h"
#include "Actor/Effects/PlayerLocus/PlayerLocusEffect.h"
#include <TutorialActor/TutorialMission/MoveMission.h>
#include <TutorialActor/TutorialMission/TutorialMissionManager.h>

TutorialScene::TutorialScene() {}
TutorialScene::~TutorialScene() {}

void TutorialScene::Init() {
    BaseScene::Init();
    GameOption::GetInstance()->Init();
    Boundary::GetInstance()->ResetHoles();

    ParticleManager::GetInstance()->SetViewProjection(&viewProjection_);
    ExpEmitter::GetInstance()->Init();

    // 生成
    skyDome_                      = std::make_unique<SkyDome>();
    fade_                         = std::make_unique<Fade>();
    player_                       = std::make_unique<Player>();
    gameCamera_                   = std::make_unique<GameCamera>();
    lockOn_                       = std::make_unique<LockOn>();
    stations_[FactionType::Ally]  = std::make_unique<PlayerStation>("PlayerStation");
    stations_[FactionType::Enemy] = std::make_unique<EnemyStation>("EnemyStation");

    /// UI -----
    miniMap_ = std::make_unique<MiniMap>();
    uis_     = std::make_unique<GameUIs>();

    /// Effect -----
    outsideWarning_    = std::make_unique<GameScreenEffect>();
    engineEffect_      = std::make_unique<PlayerEngineEffect>();
    playerLocusEffect_ = std::make_unique<PlayerLocusEffect>();

    /// Tutorial System -----
    tutorialManager_ = std::make_unique<TutorialMissionManager>();

    // 初期化
    fade_->Init();
    player_->Init();
    player_->SetWorldPositionY(100.0f);
    gameCamera_->Init();
    lockOn_->Init();
    skyDome_->Init();

    // プレイヤーを攻撃対象に追加
    auto enemyStation = dynamic_cast<EnemyStation*>(stations_[FactionType::Enemy].get());
    enemyStation->SetPlayerPtr(player_.get());

    const Vector3 enemyStationPos = stations_[FactionType::Enemy]->GetWorldPosition();

    /// UI -----
    miniMap_->RegisterPlayer(player_.get());
    uis_->Init();
    miniMap_->Init(stations_[FactionType::Ally].get(), stations_[FactionType::Enemy].get());

    /// Effect -----
    outsideWarning_->Init();
    engineEffect_->Init();
    playerLocusEffect_->Init(player_.get());

    /// Tutorial System -----
    InitializeTutorialMissions();
    tutorialManager_->Init();

    boundary_ = Boundary::GetInstance();
    boundary_->Init();

    // set
    player_->SetGameCamera(gameCamera_.get());
    player_->SetLockOn(lockOn_.get());
    engineEffect_->SetPlayer(player_.get());
    gameCamera_->SetPlayer(player_.get());
    gameCamera_->SetTarget(&player_->GetTransform());

    Frame::ResetDeltaTime();
    ViewProjectionUpdate();
}

void TutorialScene::InitializeTutorialMissions() {
    // プレイヤーをチュートリアルマネージャーに設定
    tutorialManager_->SetPlayer(player_.get());
}

void TutorialScene::Update() {
    /// オプションの開閉
    /* GameOption* op = GameOption::GetInstance();
     /// Optionを開ける条件
     if (!fade_->IsFade()) {
         /// 入力でOptionを開く
         if (input_->TrrigerKey(DIK_ESCAPE) || input_->IsTriggerPad(0, Gamepad::Start)) {
             if (!op->GetIsOpen()) {
                 op->Open();
             }
         }
     }*/

    if (input_->TrrigerKey(DIK_ESCAPE) || input_->IsTriggerPad(0, Gamepad::Start)) {
        if (!isTutorialEnd_) {
            isTutorialEnd_ = true;
            fade_->FadeOut(1.0f);
        }
    }

   
    /*   op->Update();*/
    // tutorial update
    TutorialUpdate();
    HandleSceneTransition();

    /// Scene Change
    /* if (!op->GetIsOpen()) {
         HandleSceneTransition();
     }*/

    /// フェードアウトが終わったらシーンチェンジ
    if (fade_->IsFadeEnd()) {
        SceneManager::GetInstance()->ChangeScene("GAMEPLAY");
        return;
    }
}

void TutorialScene::HandleSceneTransition() {
    // チュートリアルが完了した場合、スペースキーでゲームシーンへ
    if (tutorialManager_->IsCompleted()) {
        if (!isTutorialEnd_) {
            isTutorialEnd_ = true;
            fade_->FadeOut(1.0f);
        }
        /// 効果音の再生
        /*	int soundId = audio_->LoadWave("./resources/Sound/SE/DecideSE.wav");
            audio_->PlayWave(soundId, 0.2f);
            audio_->StopBGM(bgmId_);*/    
    } 

     // 自動的にチュートリアルを開始
    if (!tutorialManager_->IsInProgress() && !tutorialManager_->IsTransitioning() && tutorialManager_->GetStatus() == TutorialMissionManager::TutorialStatus::NOT_STARTED) {
        tutorialManager_->StartTutorial();
    }

}

void TutorialScene::TutorialUpdate() {
    fade_->Update();

    // objUpdate
    player_->Update();
    gameCamera_->Update();
    skyDome_->Update();

    Boundary::GetInstance()->Update();

    // lockOn更新
    /* lockOn_->Update(targets, player_.get(), viewProjection_, FactionType::Enemy);*/

    /// tutorial system update
    tutorialManager_->Update();

    /// ui update
    miniMap_->Update();
    uis_->Update(player_.get());

    /// effect update
    playerLocusEffect_->Update();

    Object3DRegistry::GetInstance()->UpdateAll();
    ParticleManager::GetInstance()->Update();

    Debug();
    ViewProjectionUpdate();
}

/// ===================================================
/// モデル描画
/// ===================================================
void TutorialScene::ModelDraw() {
    ID3D12GraphicsCommandList* commandList = DirectXCommon::GetInstance()->GetCommandList();

    Line3DPipeline* line3dPipeline = Line3DPipeline::GetInstance();
    line3dPipeline->PreDraw(commandList);

    /*for (auto& stations : stations_)
        stations.second->DrawDebug(viewProjection_);*/

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
    // CollisionManager::GetInstance()->Draw(viewProjection_);

    /// 境界の破片の描画
    BoundaryShardPipeline* boundaryShardPipeline = BoundaryShardPipeline::GetInstance();
    boundaryShardPipeline->PreDraw(commandList);
    boundaryShardPipeline->Draw(commandList, viewProjection_);

    /// 境界の穴の境界を描画
    BoundaryEdgePipeline* boundaryEdgePipeline = BoundaryEdgePipeline::GetInstance();
    boundaryEdgePipeline->PreDraw(commandList);
    boundaryEdgePipeline->Draw(commandList, viewProjection_);

  
}

/// ===================================================
/// SkyBox描画
/// ===================================================
void TutorialScene::SkyBoxDraw() {}

/// ===================================================
/// スプライト描画
/// ===================================================
void TutorialScene::SpriteDraw() {
    /*titleSprite_->Draw();*/

    GameOption* op = GameOption::GetInstance();
    op->Draw();

    // チュートリアルUI描画（イージングとゲージを含む）
    tutorialManager_->SpriteDraw();

    // チュートリアル完了時のメッセージ表示
    DrawTutorialCompletionMessage();

    fade_->Draw();
}

void TutorialScene::DrawTutorialCompletionMessage() {

    if (tutorialManager_->IsCompleted()) {
    }
}

/// ===================================================
/// 影
/// ===================================================
void TutorialScene::DrawShadow() {}

void TutorialScene::Debug() {
#ifdef _DEBUG
    ImGui::Begin("Camera");
    ImGui::DragFloat3("pos", &viewProjection_.translation_.x, 0.1f);
    ImGui::DragFloat3("rotate", &viewProjection_.rotation_.x, 0.1f);
    ImGui::End();

    // チュートリアルシステムのデバッグ表示
    if (tutorialManager_) {
        tutorialManager_->AdjustParam();
    }

    // シーン固有のデバッグ情報
    ImGui::Begin("Tutorial Scene Debug");
    ImGui::Text("Tutorial Status: %s",
        tutorialManager_->IsCompleted() ? "Completed" : tutorialManager_->IsInProgress()  ? "In Progress"
                                                    : tutorialManager_->IsTransitioning() ? "Transitioning"
                                                                                          : "Not Started");

    ImGui::Text("Total Progress: %.1f%%", tutorialManager_->GetTotalProgress() * 100.0f);
    ImGui::Text("Current Mission: %d / %d",
        tutorialManager_->GetCurrentMissionIndex() + 1,
        tutorialManager_->GetTotalMissions());

    if (ImGui::Button("Force Complete Current Mission")) {
        tutorialManager_->SkipCurrentMission();
    }

    ImGui::End();
#endif
}

// ビュープロジェクション更新
void TutorialScene::ViewProjectionUpdate() {
    BaseScene::ViewProjectionUpdate();
}

void TutorialScene::ViewProssess() {
    viewProjection_.matView_       = gameCamera_->GetViewProjection().matView_;
    viewProjection_.matProjection_ = gameCamera_->GetViewProjection().matProjection_;
    viewProjection_.cameraMatrix_  = gameCamera_->GetViewProjection().cameraMatrix_;
    viewProjection_.rotation_      = gameCamera_->GetViewProjection().rotation_;
    viewProjection_.TransferMatrix();
}