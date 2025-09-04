#pragma once

/// game
#include "Actor/Player/Player.h"

/// ////////////////////////////////////////////////////////////
/// プレイヤーがフィールド外に出たときの警告
/// ////////////////////////////////////////////////////////////
class PlayerOutsideWarning {
public:
	/// ==========================================================
	/// public : methods
	/// ==========================================================
	
	PlayerOutsideWarning() = default;
	~PlayerOutsideWarning() = default;
	
	void Init();
	void Update(Player* _player);
	void Draw();
	
private:
	/// ==========================================================
	/// private : objects
	/// ==========================================================

	Vector3 warningRectSize_;

};

