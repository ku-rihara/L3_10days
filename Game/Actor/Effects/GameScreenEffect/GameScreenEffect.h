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
	
	const Vector4& GetBaseColor() const;
	void SetBaseColor(const Vector4& color);

private:
	/// ==========================================================
	/// private : objects
	/// ==========================================================

	Vector4 baseColor_;

};

