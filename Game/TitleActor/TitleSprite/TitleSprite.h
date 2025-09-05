#pragma once

#include <memory>

/// engine
#include "2d/Sprite.h"

class TitleSprite {
public:
	/// ==================================
	/// public : methdos
	/// ==================================

	TitleSprite() = default;
	~TitleSprite() = default;

	void Init();
	void Update();
	void Draw();

private:
	/// ==================================
	/// private : objects
	/// ==================================

	std::unique_ptr<Sprite> sprite_ = nullptr;


};

