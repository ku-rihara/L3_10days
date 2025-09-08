#include "GameController.h"

/// engine
#include "Frame/Frame.h"

/// game
#include "Actor/Player/Player.h"
#include "Actor/Station/Base/BaseStation.h"

GameController::GameController() {
	/// NumDraw
	outOfFieldWarningTimeIntNumDraw_ = std::make_unique<NumDraw>();
	outOfFieldWarningTimeFracNumDraw_ = std::make_unique<NumDraw>();

	outOfFieldWarningTimeIntNumDraw_->Init(3);
	outOfFieldWarningTimeFracNumDraw_->Init(3);

	float scale = 2.0f;
	outOfFieldWarningTimeIntNumDraw_->SetScale({ scale, scale });
	outOfFieldWarningTimeFracNumDraw_->SetScale({ scale, scale });

	/// 二つの数字の間隔
	Vector2 offset = { 10.0f, 0.0f };
	outOfFieldWarningTimeIntNumDraw_->SetBasePosition({ 1280.0f - 200.0f - offset.x, 50.0f });
	outOfFieldWarningTimeFracNumDraw_->SetBasePosition({ 1280.0f - 200.0f + offset.x, 50.0f });

	outOfFieldWarningTimeIntNumDraw_->SetDigitSpacing(12.0f);
	outOfFieldWarningTimeFracNumDraw_->SetDigitSpacing(12.0f);

}
GameController::~GameController() = default;

void GameController::Update() {
	isGameClear_ = CheckIsGameClear();
	isGameOver_ = CheckIsGameOver();
	isPlayerOutOfField_ = CheckIsPlayerOutOfField();

	if (isPlayerOutOfField_) {
		outOfFieldTime_ += Frame::DeltaTime();
	}


	/// 現在の経過時間を整数部分と小数部分に分ける
	float remainingTime = kMaxOutOfFieldTime_ - outOfFieldTime_;
	if (remainingTime < 0.0f) remainingTime = 0.0f;
	int intPart = static_cast<int>(remainingTime);
	int fracPart = static_cast<int>((remainingTime - intPart) * 100.0f); // 小数点以下2桁
	outOfFieldWarningTimeIntNumDraw_->SetNumber(intPart);
	outOfFieldWarningTimeFracNumDraw_->SetNumber(fracPart);

	outOfFieldWarningTimeIntNumDraw_->Update();
	outOfFieldWarningTimeFracNumDraw_->Update();
}

void GameController::DrawOutOfFieldWarningTime() {

	outOfFieldWarningTimeIntNumDraw_->Draw();
	outOfFieldWarningTimeFracNumDraw_->Draw();
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