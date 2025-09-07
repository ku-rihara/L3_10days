#pragma once

/// std
#include <memory>

/// engine
#include "2d/Sprite.h"

class GameClearSprite {
public:
	GameClearSprite();
	~GameClearSprite();
	void Init();
	void Update();
	void Draw();

private:
	std::unique_ptr<Sprite> sprite_; ///< スプライト
};

