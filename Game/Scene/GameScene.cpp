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

#include <imgui.h>

GameScene::GameScene() {}

GameScene::~GameScene() {
}

void GameScene::Init() {

    BaseScene::Init();

    // 生成
    skuBox_ = std::make_unique<SkyBox>();
    player_ = std::make_unique<Player>();
    enemyStation_ = std::make_unique<EnemyStation>("EnemyStation");
    
    // 初期化
    skuBox_->Init();
    player_->Init();
    enemyStation_->Init();

    //ParticleViewSet
    ParticleManager::GetInstance()->SetViewProjection(&viewProjection_);
}

void GameScene::Update() {

    /// debugCamera
    debugCamera_->Update();
    Debug();

    // class Update
    player_->Update();
    enemyStation_->Update();

    // obj3Dies AllUpdate
    Object3DRegistry::GetInstance()->UpdateAll();
    AnimationRegistry::GetInstance()->UpdateAll(Frame::DeltaTimeRate());

 
    // viewProjection 更新
    ViewProjectionUpdate();

    // Scene Change
    if (input_->TrrigerKey(DIK_RETURN)) {
        SceneManager::GetInstance()->ChangeScene("TITLE");
    }

    //Particle AllUpdate
    ParticleManager::GetInstance()->Update();
}

/// ===================================================
/// モデル描画
/// ===================================================
void GameScene::ModelDraw() {
   
    ID3D12GraphicsCommandList* commandList = DirectXCommon::GetInstance()->GetCommandList();
    Object3DPiprline::GetInstance()->PreDraw(commandList);

    // Model AllUpdate
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
void GameScene::SpriteDraw() {
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
   
    ShadowMap::GetInstance()->DebugImGui();

    enemyStation_->ShowGui();

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
