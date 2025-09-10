#include "GameClearEffect.h"

/// engine
#include "Frame/Frame.h"

GameClearEffect::GameClearEffect() = default;
GameClearEffect::~GameClearEffect() = default;

void GameClearEffect::Init() {
	isStart_ = false;
	isEnd_ = false;
	timer_ = 0.0f;
}

void GameClearEffect::Update() {
	if (!isStart_ || isEnd_) {
		return;
	}

	timer_ += Frame::DeltaTime();
	if (timer_ >= effectTime_) {
		isEnd_ = true;
	}
}

void GameClearEffect::Draw() {
	if (!isStart_ || isEnd_) {
		return;
	}
}

void GameClearEffect::Start() {
	isStart_ = true;
	isEnd_ = false;
	timer_ = 0.0f;
}

bool GameClearEffect::GetIsEnd() {
	return isEnd_;
}

