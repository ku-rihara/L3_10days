#include "GameController.h"

/// engine
#include "Frame/Frame.h"

/// game
#include "Actor/Player/Player.h"
#include "Actor/Station/Base/BaseStation.h"

GameController::GameController() = default;
GameController::~GameController() = default;

void GameController::Update() {
	isGameClear_ = CheckIsGameClear();
	isGameOver_ = CheckIsGameOver();
	isPlayerOutOfField_ = CheckIsPlayerOutOfField();

	if (isPlayerOutOfField_) {
		outOfFieldTime_ += Frame::DeltaTime();
	}
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
	if (outOfFieldTime_ >= kMaxOutOfFieldTime_) {
		result = true;
	}

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

bool GameController::CheckIsPlayerOutOfField() {
	/// 中心から一定距離以上離れたらout of field扱い
	playerToCenterDistance_ = player_->GetWorldPosition().Length();
	if (playerToCenterDistance_ > kMaxPlayerToCenterDistance_) {
		outOfFieldTime_ += Frame::DeltaTime();
		return true;
	}

	outOfFieldTime_ = 0.0f;
	return false;
}

bool GameController::GetIsGameOver() const {
	return isGameOver_;
}

bool GameController::GetIsGameClear() const {
	return isGameClear_;
}

bool GameController::GetIsPlayerOutOfField() const {
	return isPlayerOutOfField_;
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