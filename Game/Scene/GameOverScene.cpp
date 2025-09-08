#include "GameOverScene.h"

#include "Scene/Manager/SceneManager.h"

/// actors
#include "GameOverActor/GameOverSprite.h"

GameOverScene::GameOverScene() = default;
GameOverScene::~GameOverScene() = default;

void GameOverScene::Init() {
	BaseScene::Init();

	/// インスタンスの生成
	gameOverSprite_ = std::make_unique<GameOverSprite>();


	/// 各種初期化
	gameOverSprite_->Init();

}
void GameOverScene::Update() {
	gameOverSprite_->Update();


	/// 選択したときの処理
	if (input_->TrrigerKey(DIK_RETURN) ||
		input_->TrrigerKey(DIK_SPACE) ||
		input_->IsTriggerPad(0, Gamepad::A)) {

		switch (gameOverSprite_->GetSelectIndex()) {
		case GameOverSprite::RetryGame:
			SceneManager::GetInstance()->ChangeScene("GAMEPLAY");
			return;
		case GameOverSprite::ReturnTitle:
			SceneManager::GetInstance()->ChangeScene("TITLE");
			return;
		}
	}

}

void GameOverScene::ModelDraw() {}

void GameOverScene::SpriteDraw() {
	gameOverSprite_->Draw();
}

void GameOverScene::SkyBoxDraw() {}

void GameOverScene::DrawShadow() {}

void GameOverScene::Debug() {}

void GameOverScene::ViewProjectionUpdate() {}

void GameOverScene::ViewProssess() {}

