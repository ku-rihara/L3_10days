#pragma once

/// game
#include "Actor/Player/Player.h"

/// ////////////////////////////////////////////////////////////
/// プレイヤーがフィールド外に出たときの警告
/// ////////////////////////////////////////////////////////////
class GameScreenEffect {
public:
	/// ==========================================================
	/// public : methods
	/// ==========================================================
	
	GameScreenEffect() = default;
	~GameScreenEffect() = default;
	
	void Init();
	void Update(Player* _player);
	void Draw();
	
private:
	/// ==========================================================
	/// private : objects
	/// ==========================================================

	Vector3 warningRectSize_;

};

