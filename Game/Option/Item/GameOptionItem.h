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
	void BaseUpdate(size_t _currentIndex);
	void BaseDraw();

	virtual void Init() = 0;
	virtual void Update(size_t _currentIndex, bool _isDirtyThisFrame) = 0;
	virtual void Draw() = 0;

	const Vector2& GetStartPos() const;
	const Vector2& GetOffsetPos() const;

private:
	/// ==================================
	/// private : methods
	/// ==================================

	std::unique_ptr<Sprite> textSprite_ = nullptr;

	Vector2 offsetPos_ = { 0.0f, 80.0f };
	Vector2 startPos_ = { 360.0f, 240.0f };

	float scale_ = 1.0f;       // 拡大率
	float rotation_ = 0.0f;    // 回転角
	float colorScale_ = 1.0f;  // 色の強さ
	bool isSelected_ = false;  // 選択されているか
	bool isActive_ = true;     // 使用可能か


protected:
	/// ==================================
	/// protected : objects
	/// ==================================

	size_t thisIndex_ = 0;     // 自分のインデックス
};

