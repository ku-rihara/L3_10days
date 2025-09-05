/// scene
#include "TitleScene.h"
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

/// game
#include "TitleActor/FighterAircraft/FighterAircraft.h"

TitleScene::TitleScene() {}
TitleScene::~TitleScene() {}

void TitleScene::Init() {
	BaseScene::Init();

	// 生成
	for (size_t i = 0; i < 16; i++) {
		Vector3 pos = {
			static_cast<float>(i) * 16.0f,
			0.0f, 0.0f
		};
		object3ds_.push_back(std::make_unique<FighterAircraft>(pos));
	}

	// 初期化
	for (auto& obj : object3ds_) {
		obj->Init();
	}



	titleSprite_ = std::make_unique<TitleSprite>();
	titleSprite_->Init();

	ParticleManager::GetInstance()->SetViewProjection(&viewProjection_);
}

void TitleScene::Update() {

	for (auto& obj : object3ds_) {
		obj->Update();
	}

	titleSprite_->Update();

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
void TitleScene::ModelDraw() {
	/// commandList取得
	ID3D12GraphicsCommandList* commandList = DirectXCommon::GetInstance()->GetCommandList();
	Object3DPiprline::GetInstance()->PreDraw(commandList);
	Object3DRegistry::GetInstance()->DrawAll(viewProjection_);
	ParticleManager::GetInstance()->Draw(viewProjection_);
}

/// ===================================================
/// SkyBox描画
/// ===================================================
void TitleScene::SkyBoxDraw() {}

/// ===================================================
/// スプライト描画
/// ===================================================
void TitleScene::SpriteDraw() {
	titleSprite_->Draw();
}

/// ===================================================
/// 影
/// ===================================================
void TitleScene::DrawShadow() {}

void TitleScene::Debug() {
#ifdef _DEBUG
	ImGui::Begin("Camera");
	ImGui::DragFloat3("pos", &viewProjection_.translation_.x, 0.1f);
	ImGui::DragFloat3("rotate", &viewProjection_.rotation_.x, 0.1f);
	ImGui::End();
#endif
}

// ビュープロジェクション更新
void TitleScene::ViewProjectionUpdate() {
	BaseScene::ViewProjectionUpdate();
}

void TitleScene::ViewProssess() {
	viewProjection_.UpdateMatrix();
}
