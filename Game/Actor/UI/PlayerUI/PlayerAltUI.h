#pragma once

#include <memory>

#include "2d/Sprite.h"
#include "../NumDraw/NumDraw.h"

class PlayerAltUI {
public:
	PlayerAltUI()  = default;
	~PlayerAltUI() = default;

	void Init();
	void Update(class Player* _player);
	void Draw();

private:
	std::unique_ptr<Sprite> altUISprite_;
	std::unique_ptr<NumDraw> altNumDraw_;
};

