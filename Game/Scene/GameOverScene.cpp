#include "GameOverScene.h"

/// actors
#include "GameOverActor/GameOverSprite.h"

GameOverScene::GameOverScene() = default;
GameOverScene::~GameOverScene() = default;

void GameOverScene::Init() {

	/// インスタンスの生成
	gameOverSprite_ = std::make_unique<GameOverSprite>();


	/// 各種初期化
	gameOverSprite_->Init();

}
void GameOverScene::Update() {

	gameOverSprite_->Update();
}

void GameOverScene::ModelDraw() {
}

void GameOverScene::SpriteDraw() {
	gameOverSprite_->Draw();
}

void GameOverScene::SkyBoxDraw() {
}

void GameOverScene::DrawShadow() {
}

void GameOverScene::Debug() {
}

void GameOverScene::ViewProjectionUpdate() {
}

void GameOverScene::ViewProssess() {
}

