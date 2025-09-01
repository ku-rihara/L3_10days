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

    ground_          = std::make_unique<Ground>();
    monsterBall_     = std::make_unique<MonsterBall>();
    plane_           = std::make_unique<Plane>();
    skuBox_          = std::make_unique<SkyBox>();
    putObjForBlender = std::make_unique<PutObjForBlender>();
    cameraEditor_    = std::make_unique<CameraEditor>();
    shakeEditor_     = std::make_unique<ShakeEditor>();
    timeScaleController_ = std::make_unique<TimeScaleController>();

    monsterBall_->Init();
    ground_->Init();
    plane_->Init();
    skuBox_->Init();
    cameraEditor_->Init(&viewProjection_);
    timeScaleController_->Init();

    shakeEditor_->Init();
    putObjForBlender->LoadJsonFile("game.json");
    putObjForBlender->EasingAllReset();

  
    isDebugCameraActive_ = true;
    ParticleManager::GetInstance()->SetViewProjection(&viewProjection_);
}

void GameScene::Update() {

    /// debugCamera
    debugCamera_->Update();
    Debug();

    // 各クラス更新
    ground_->Update();
    cameraEditor_->Update(Frame::DeltaTime());
    shakeEditor_->Update(Frame::DeltaTime());
    timeScaleController_->Update(Frame::DeltaTime());
    monsterBall_->Update();
    plane_->Update();
    skuBox_->Update();

    Object3DRegistry::GetInstance()->UpdateAll();
    AnimationRegistry::GetInstance()->UpdateAll(Frame::DeltaTimeRate());

    putObjForBlender->EasingUpdateSelectGroup(Frame::DeltaTime(), 0);

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
    /// commandList取得
    ID3D12GraphicsCommandList* commandList = DirectXCommon::GetInstance()->GetCommandList();
    Object3DPiprline::GetInstance()->PreDraw(commandList);

    /* ground_->Draw(viewProjection_);
     plane_->Draw(viewProjection_);*/
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

    Light::GetInstance()->DebugImGui();
    ImGui::Begin("Object");
    ground_->Debug();
    monsterBall_->Debug();
    plane_->Debug();
    skuBox_->Debug();
    ShadowMap::GetInstance()->DebugImGui();

    ImGui::End();
    cameraEditor_->EditorUpdate();
    shakeEditor_->EditorUpdate();
    timeScaleController_->EditorUpdate();
#endif
}

// ビュープロジェクション更新
void GameScene::ViewProjectionUpdate() {
    BaseScene::ViewProjectionUpdate();
}

void GameScene::ViewProssess() {
    viewProjection_.UpdateMatrix();
}
