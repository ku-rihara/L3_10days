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
#include "Option/GameOption.h"

TitleScene::TitleScene() {}
TitleScene::~TitleScene() {
}

void TitleScene::Init() {
	BaseScene::Init();
	GameOption::GetInstance()->Init();

	// 生成
	object3ds_.push_back(std::make_unique<FighterAircraft>(Vector3{}));

	// 初期化
	for (auto& obj : object3ds_) {
		obj->Init();
	}


	titleSprite_ = std::make_unique<TitleSprite>();
	titleSprite_->Init();

	/// カメラの位置を調整
	//viewProjection_.Init();
	viewProjection_.translation_ = Vector3(4.02f, 2.61f, -7.57f);
	viewProjection_.rotation_ = Vector3(0.24f, -0.55f, 0.0f);

	ParticleManager::GetInstance()->SetViewProjection(&viewProjection_);

	/// このシーンのBGMを再生
	bgmId_ = audio_->LoadWave("./resources/Sound/the_tmp.wav");
	audio_->PlayBGM(bgmId_, 0.1f);


	fade_ = std::make_unique<Fade>();
	fade_->Init();


	Frame::ResetDeltaTime();
	ViewProjectionUpdate();
}

void TitleScene::Update() {

	if(input_->PushKey(DIK_ESCAPE)){
		int soundId = audio_->LoadWave("./resources/Sound/the_tmp.wav");
		audio_->StopBGM(soundId);
	}

	for (auto& obj : object3ds_) {
		obj->Update();
	}

	titleSprite_->Update();
	fade_->Update();

	if(input_->TrrigerKey(DIK_F)) {
		fade_->FadeOut(0.02f);
	}

	Object3DRegistry::GetInstance()->UpdateAll();
	ParticleManager::GetInstance()->Update();

	Debug();
	ViewProjectionUpdate();


	/// Scene Change
	if(input_->TrrigerKey(DIK_SPACE)){
		/// 効果音の再生
		int soundId = audio_->LoadWave("./resources/Sound/the_tmp.wav");
		audio_->PlayWave(soundId, 0.2f);
		audio_->StopBGM(bgmId_);

		/// 一旦直接変更するが、あとでフェードをかけるのと、シーンをゲームスタートシーンにする
		SceneManager::GetInstance()->ChangeScene("GAMEPLAY");
		return;
	}


	/// Debug用なのであとで消す
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

	fade_->Draw();
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
