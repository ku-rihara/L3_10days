#include "GameClearScene.h"

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
}

void GameClearScene::ModelDraw() {
}

void GameClearScene::SpriteDraw() {
	gameClearSprite_->Draw();
}

void GameClearScene::SkyBoxDraw() {
}

void GameClearScene::DrawShadow() {
}

void GameClearScene::Debug() {
}

void GameClearScene::ViewProjectionUpdate() {
}

void GameClearScene::ViewProssess() {
}

