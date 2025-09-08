#pragma once

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

public:
	/// getter
	bool GetIsGameOver() const;
	bool GetIsGameClear() const;
	bool GetIsPlayerOutOfField() const;

	void SetPlayer(Player* _player);
	void SetEnemyStation(BaseStation* _station);
	void SetPlayerStation(BaseStation* _station);
};

