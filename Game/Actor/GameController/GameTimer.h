#pragma once

/// std
#include <memory>
#include <vector>

/// game
#include "Actor/UI/NumDraw/NumDraw.h"

class GameTimer {
public:
	enum {
		MINUTES,
		SECONDS,
		FRACTIONS
	};
public:
	GameTimer();
	~GameTimer();
	void Init();
	void Update();
	void Draw();
private:
	using USprite = std::unique_ptr<Sprite>;
	using UNumDraw = std::unique_ptr<NumDraw>;
	std::vector<UNumDraw> numDraws_;
	std::vector<USprite> colons_;
	float time_;
};

