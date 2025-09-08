#include "GameController.h"

/// game
#include "Actor/Player/Player.h"
#include "Actor/Station/Base/BaseStation.h"

GameController::GameController() = default;
GameController::~GameController() = default;

void GameController::Update() {
	isGameClear_ = CheckIsGameClear();
	isGameOver_ = CheckIsGameOver();
}

bool GameController::CheckIsGameOver() {
	bool result = false;
	/// PlayerがHP0以下になったか
	if (player_->GetHP() <= 0.0f) {
		result = true;
	}

	/// 敵に基地が破壊されたか
	if (playerStation_->GetHp() <= 0.0f) {
		result = true;
	}

	/// Playerがフィールド外に一定時間以上出たか
	/// TODO: 実装

	return result;
}

bool GameController::CheckIsGameClear() {
	bool result = false;

	/// 敵の基地が破壊されたか
	if (enemyStation_->GetHp() <= 0.0f) {
		result = true;
	}

	return result;
}

bool GameController::GetIsGameOver() const {
	return isGameOver_;
}

bool GameController::GetIsGameClear() const {
	return isGameClear_;
}


void GameController::SetPlayer(Player* player) {
	player_ = player;
}

void GameController::SetPlayerStation(BaseStation* station) {
	playerStation_ = station;
}

void GameController::SetEnemyStation(BaseStation* station) {
	enemyStation_ = station;
}