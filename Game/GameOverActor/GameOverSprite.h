#pragma once

/// std
#include <memory>
#include <vector>

/// engine
#include "2d/Sprite.h"

/// ////////////////////////////////////////////////////////////////////
/// @brief  ゲームオーバースプライト
/// ////////////////////////////////////////////////////////////////////
class GameOverSprite {
public:
	using USprite = std::unique_ptr<Sprite>;

public:
	/// ==============================================================
	/// public : methods
	/// ==============================================================

	GameOverSprite();
	~GameOverSprite();

	void Init();
	void Update();
	void Draw();


private:
	/// ==============================================================
	/// private : methods
	/// ==============================================================

	USprite background_;
	USprite gameOverText_;
	USprite itemFrame_;
	USprite selectedFrame_;

	std::vector<USprite> itemIcons_;
	size_t selectIndex_ = 0;

};

