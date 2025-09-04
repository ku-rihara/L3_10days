#include "GameUIs.h"


void GameUIs::Init() {
	playerSpeedUI_ = std::make_unique<PlayerSpeedUI>();
	playerAltUI_ = std::make_unique<PlayerAltUI>();

	playerSpeedUI_->Init();
	playerAltUI_->Init();
}

void GameUIs::Update() {
	playerSpeedUI_->Update();
	playerAltUI_->Update();
}

void GameUIs::Draw() {
	playerSpeedUI_->Draw();
	playerAltUI_->Draw();
}