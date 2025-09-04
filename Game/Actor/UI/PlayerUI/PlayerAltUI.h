#pragma once

#include <memory>

#include "2d/Sprite.h"

class PlayerAltUI {
public:
	PlayerAltUI()  = default;
	~PlayerAltUI() = default;
	void Init();
	void Update();
	void Draw();

private:
	std::unique_ptr<Sprite> altUISprite_;
};

