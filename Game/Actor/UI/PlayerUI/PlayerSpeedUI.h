#pragma once

#include <memory>

#include "2d/Sprite.h"
#include "../NumDraw/NumDraw.h"

class PlayerSpeedUI {
public:
	PlayerSpeedUI()  = default;
	~PlayerSpeedUI() = default;

	void Init();
	void Update(class Player* _player);
	void Draw();

private:

	std::unique_ptr<Sprite> speedUISprite_;
	std::unique_ptr<NumDraw> speedNumDraw_;

};

