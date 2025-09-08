#include "GameScore.h"

GameScore::GameScore() = default;
GameScore::~GameScore() = default;

GameScore* GameScore::GetInstance() {
	static GameScore instance;
	return &instance;
}

void GameScore::ScoreReset() {
	breakEnemyCount_ = 0;
	clearTime_ = 0.0f;
}

void GameScore::AddBreakEnemyCount(int count) {
	breakEnemyCount_ += count;
}

void GameScore::SetClearTime(float time) {
	clearTime_ = time;
}

int GameScore::GetBreakEnemyCount() const {
	return breakEnemyCount_;
}

float GameScore::GetClearTime() const {
	return clearTime_;
}

