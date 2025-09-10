#pragma once

/// std
#include <memory>
#include <vector>

/// game
#include "Actor/UI/NumDraw/NumDraw.h"

class GameTimer {
public:
	GameTimer();
	~GameTimer();
	void Init();
	void Update(bool _isGameClear);
	void Draw();
	float GetTime() const;
private:
	using USprite = std::unique_ptr<Sprite>;
	using UNumDraw = std::unique_ptr<NumDraw>;
	std::vector<UNumDraw> numDraws_;
	std::vector<USprite> colons_;
	float time_;
};

