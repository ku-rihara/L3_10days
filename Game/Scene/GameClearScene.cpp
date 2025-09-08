#include "GameClearScene.h"

/// engine
#include "Scene/Manager/SceneManager.h"

/// actors
#include "GameClearActor/GameClearSprite.h"

GameClearScene::GameClearScene() = default;
GameClearScene::~GameClearScene() = default;

void GameClearScene::Init() {
	BaseScene::Init();

	/// インスタンスの生成
	gameClearSprite_ = std::make_unique<GameClearSprite>();

	/// 各種初期化
	gameClearSprite_->Init();
}

void GameClearScene::Update() {
	gameClearSprite_->Update();

	/// タイトルに戻る処理
	if (input_->TrrigerKey(DIK_RETURN) ||
		input_->TrrigerKey(DIK_SPACE) ||
		input_->IsTriggerPad(0, Gamepad::A)) {
		SceneManager::GetInstance()->ChangeScene("TITLE");
		return;
	}

}

void GameClearScene::ModelDraw() {}

void GameClearScene::SpriteDraw() {
	gameClearSprite_->Draw();
}

void GameClearScene::SkyBoxDraw() {}

void GameClearScene::DrawShadow() {}

void GameClearScene::Debug() {}

void GameClearScene::ViewProjectionUpdate() {}

void GameClearScene::ViewProssess() {}

