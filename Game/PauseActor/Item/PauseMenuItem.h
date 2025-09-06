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

	void SetIsSelected(bool _isSelected);
	void SetIsActive(bool _isActive);
	void SetScale(float _scale);
	void SetRotation(float _rotation);

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
	Vector3 baseColor_ = { 1.0f, 1.0f, 1.0f };
	float   colorScale_ = 1.0f;

	size_t thisIndex_ = 0;   // 自分のインデックス

	float minScale_ = 0.5f;
	float maxScale_ = 1.2f;

};

