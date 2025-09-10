#pragma once

/// std
#include <memory>

/// engine
#include "2d/Sprite.h"

/// game
#include "Actor/UI/NumDraw/NumDraw.h"

/// 前方宣言
class Player;
class BaseStation;

/// //////////////////////////////////////////////////////////////////
/// ゲームの状態管理
/// //////////////////////////////////////////////////////////////////
class GameController {
public:
	GameController();
	~GameController();

	/// 更新
	void Update();

	/// 残り時間の表示
	void DrawOutOfFieldWarningTime();
	void DrawGameTimer();

	/// チェック
	bool CheckIsGameOver();
	bool CheckIsGameClear();
	bool CheckIsPlayerOutOfField();

private:
	/// ゲームの状態
	bool isGameOver_ = false;
	bool isGameClear_ = false;

	/// other classes
	Player* player_ = nullptr;
	BaseStation* enemyStation_ = nullptr;
	BaseStation* playerStation_ = nullptr;

	/// Playerがフィールド外に出ている時間
	float outOfFieldTime_ = 0.0f;
	const float kMaxOutOfFieldTime_ = 10.0f;// n秒でゲームオーバー
	float playerToCenterDistance_ = 0.0f;
	const float kMaxPlayerToCenterDistance_ = 4000.0f;// 中心からn以上離れたらout of field扱い
	bool isPlayerOutOfField_ = false;
	std::unique_ptr<NumDraw> outOfFieldWarningTimeIntNumDraw_;   // 整数部分
	std::unique_ptr<NumDraw> outOfFieldWarningTimeFracNumDraw_;  // 小数部分
	std::unique_ptr<Sprite> commaSprite_; // 小数点

	/// ゲームタイマー
	std::unique_ptr<class GameTimer> gameTimer_;

public:
	/// getter
	bool GetIsGameOver() const;
	bool GetIsGameClear() const;
	bool GetIsPlayerOutOfField() const;

	void SetPlayer(Player* _player);
	void SetEnemyStation(BaseStation* _station);
	void SetPlayerStation(BaseStation* _station);
};

