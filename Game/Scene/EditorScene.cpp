/// scene
#include "EditorScene.h"
#include "GameScene.h"
#include "Manager/SceneManager.h"

#include "base/TextureManager.h"
// class
#include "3d/Object3DRegistry.h"
#include "Pipeline/Object3DPiprline.h"
#include "utility/ParticleEditor/ParticleManager.h"

// math
#include "Frame/Frame.h"
#include <imgui.h>

EditorScene::EditorScene() {}

EditorScene::~EditorScene() {
}

void EditorScene::Init() {

    BaseScene::Init();
    easingTestObject_ = std::make_unique<EasingTestObj>();
    easingTestObject_->Init();

    easingEditor_.Init();

    // Particle初期化(ファイル名,形状,Particle数上限)
    testEmitter_[0].reset(ParticleEmitter::CreateParticlePrimitive("test1", PrimitiveType::Sphere, 500));
    testEmitter_[1].reset(ParticleEmitter::CreateParticlePrimitive("test2", PrimitiveType::Plane, 500));
    testEmitter_[2].reset(ParticleEmitter::CreateParticlePrimitive("test3", PrimitiveType::Plane, 500));

    easingEditor_.SetVector3Target(&easingTestObject_->GetEasingData());
    ParticleManager::GetInstance()->SetViewProjection(&viewProjection_);
}

void EditorScene::Update() {

    // Particle更新
    for (int i = 0; i < testEmitter_.size(); i++) {
        testEmitter_[i]->Update(); // 更新
        testEmitter_[i]->EditorUpdate(); // パラメータ編集
        testEmitter_[i]->Emit(); // 発射

        /*   if (Input::GetInstance()->TrrigerKey(DIK_O)) {
               testEmitter_[i]->Emit();
           }*/
    }

    easingEditor_.Edit();
    easingTestObject_->Update();

    Object3DRegistry::GetInstance()->UpdateAll();
    ParticleManager::GetInstance()->Update();

    Debug();
    ViewProjectionUpdate();

    if (input_->TrrigerKey(DIK_RETURN)) {

        SceneManager::GetInstance()->ChangeScene("GAMEPLAY");
    }
}

/// ===================================================
/// モデル描画
/// ===================================================
void EditorScene::ModelDraw() {
    /// commandList取得
    ID3D12GraphicsCommandList* commandList = DirectXCommon::GetInstance()->GetCommandList();
    Object3DPiprline::GetInstance()->PreDraw(commandList);
    Object3DRegistry::GetInstance()->DrawAll(viewProjection_);

    ParticleManager::GetInstance()->Draw(viewProjection_);
}

/// ===================================================
/// SkyBox描画
/// ===================================================
void EditorScene::SkyBoxDraw() {
}

/// ===================================================
/// スプライト描画
/// ===================================================
void EditorScene::SpriteDraw() {
}

/// ===================================================
/// 影
/// ===================================================
void EditorScene::DrawShadow() {
}

void EditorScene::Debug() {
#ifdef _DEBUG
    ImGui::Begin("Camera");
    ImGui::DragFloat3("pos", &viewProjection_.translation_.x, 0.1f);
    ImGui::DragFloat3("rotate", &viewProjection_.rotation_.x, 0.1f);
    ImGui::End();

    easingTestObject_->Debug();
#endif
}

// ビュープロジェクション更新
void EditorScene::ViewProjectionUpdate() {
    BaseScene::ViewProjectionUpdate();
}

void EditorScene::ViewProssess() {
    viewProjection_.UpdateMatrix();
}
