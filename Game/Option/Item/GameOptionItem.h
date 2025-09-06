#pragma once

/// std
#include <string>
#include <memory>

/// engine
#include "2d/Sprite.h"

/// ///////////////////////////////////////////////////////
/// ゲームオプションのアイテム
/// ///////////////////////////////////////////////////////
class IGameOptionItem {
public:
	/// ==================================
	/// public : methods
	/// ==================================
	IGameOptionItem() = default;
	virtual ~IGameOptionItem() = default;

	void BaseInit(const std::string& _texFilepath, size_t _thisIndex);

	virtual void Init() = 0;
	virtual void Update(size_t _currentIndex) = 0;
	virtual void Draw() = 0;

protected:
	/// ==================================
	/// private : objects
	/// ==================================

	std::unique_ptr<Sprite> sprite_ = nullptr;

	size_t thisIndex_ = 0;     // 自分のインデックス
	float scale_ = 1.0f;       // 拡大率
	float rotation_ = 0.0f;    // 回転角
	float alpha_ = 1.0f;       // 透明度
	bool isSelected_ = false;  // 選択されているか
	bool isActive_ = true;     // 使用可能か
};

