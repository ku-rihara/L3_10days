#include "GameController.h"

/// engine
#include "Frame/Frame.h"
#include "base/TextureManager.h"

/// game
#include "Actor/Player/Player.h"
#include "Actor/Station/Base/BaseStation.h"

GameController::GameController() {
	/// NumDraw
	outOfFieldWarningTimeIntNumDraw_ = std::make_unique<NumDraw>();
	outOfFieldWarningTimeFracNumDraw_ = std::make_unique<NumDraw>();

	outOfFieldWarningTimeIntNumDraw_->Init(2);
	outOfFieldWarningTimeFracNumDraw_->Init(2);

	float scale = 3.0f;
	outOfFieldWarningTimeIntNumDraw_->SetScale({ scale, scale });
	outOfFieldWarningTimeFracNumDraw_->SetScale({ scale, scale });

	/// 二つの数字の間隔
	Vector2 offset = { 28.0f, 0.0f };
	Vector2 basePos = { 640.0f, 360.0f - 30.0f };
	outOfFieldWarningTimeIntNumDraw_->SetBasePosition(basePos - offset);
	outOfFieldWarningTimeFracNumDraw_->SetBasePosition(basePos + offset);

	float spacing = 20.0f;
	outOfFieldWarningTimeIntNumDraw_->SetDigitSpacing(spacing);
	outOfFieldWarningTimeFracNumDraw_->SetDigitSpacing(spacing);

	outOfFieldWarningTimeFracNumDraw_->SetIsDrawAll(true);

	outOfFieldWarningTimeIntNumDraw_->SetColor(Vector4(0.56f, 0.12f, 0.09f, 1.0f));
	outOfFieldWarningTimeFracNumDraw_->SetColor(Vector4(0.56f, 0.12f, 0.09f, 1.0f));

	/// カンマ
	uint32_t textureHandle = TextureManager::GetInstance()->LoadTexture("./resources/Texture/UI/Comma.png");
	commaSprite_.reset(Sprite::Create(textureHandle, basePos + Vector2(0, 4.0f), {1, 1, 1, 1}));
	commaSprite_->anchorPoint_ = { 0.5f, 0.5f };
	Vector2 texSize = commaSprite_->GetTextureSize();
	Vector2 size    = Vector2{ 32.0f, 32.0f };
	commaSprite_->SetScale(size / texSize);

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

	/// フィールド外に出ているときだけ表示
	if (!isPlayerOutOfField_) {
		return;
	}

	outOfFieldWarningTimeIntNumDraw_->Draw();
	outOfFieldWarningTimeFracNumDraw_->Draw();
	commaSprite_->Draw();
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