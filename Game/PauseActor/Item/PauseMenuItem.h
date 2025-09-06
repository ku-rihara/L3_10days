#pragma once

/// std
#include <string>
#include <memory>

/// engine
#include "2d/Sprite.h"

/// /////////////////////////////////////////////////////
/// ポーズ画面のアイテム
/// /////////////////////////////////////////////////////
class PauseMenuItem {
public:
	/// ==================================
	/// public : methods
	/// ==================================
	PauseMenuItem(const std::string& _texFilePath, size_t _thisIndex);
	~PauseMenuItem() = default;

	void Update(size_t _currentIndex);
	void Draw();

	/// ==================================
	/// getter setter
	/// ==================================

	bool GetIsSelected() const;
	bool GetIsActive() const;
	float GetScale() const;
	float GetRotation() const;
	float GetAlpha() const;

	void SetIsSelected(bool _isSelected);
	void SetIsActive(bool _isActive);
	void SetScale(float _scale);
	void SetRotation(float _rotation);
	void SetAlpha(float _alpha);

	void SetPosition(const Vector2& _pos);

private:
	/// ==================================
	/// private : objects
	/// ==================================

	std::unique_ptr<Sprite> sprite_ = nullptr;
	std::string textureFilePath_;

	bool isSelected_ = false;  // 選択されているか
	bool isActive_ = true;     // 使用可能か
	float scale_ = 1.0f;       // 拡大率
	float rotation_ = 0.0f;    // 回転角
	float alpha_ = 1.0f;       // 透明度

	size_t thisIndex_ = 0;   // 自分のインデックス

};

