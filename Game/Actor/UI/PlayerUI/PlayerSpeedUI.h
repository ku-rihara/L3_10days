#pragma once

#include <memory>

#include "2d/Sprite.h"

class PlayerSpeedUI {
public:
	PlayerSpeedUI()  = default;
	~PlayerSpeedUI() = default;

	void Init();
	void Update();
	void Draw();

private:

	std::unique_ptr<Sprite> speedUISprite_;

};

