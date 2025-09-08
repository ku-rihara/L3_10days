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

private:
	/// ゲームの状態
	bool isGameOver_ = false;
	bool isGameClear_ = false;

	/// other classes
	Player* player_ = nullptr;
	BaseStation* enemyStation_ = nullptr;
	BaseStation* playerStation_ = nullptr;

public:
	/// getter
	bool GetIsGameOver() const;
	bool GetIsGameClear() const;

	void SetPlayer(Player* _player);
	void SetEnemyStation(BaseStation* _station);
	void SetPlayerStation(BaseStation* _station);
};

