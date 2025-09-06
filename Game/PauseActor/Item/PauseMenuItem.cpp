#include "PauseMenuItem.h"

/// engine
#include "base/TextureManager.h"

PauseMenuItem::PauseMenuItem(const std::string& _texFilePath, size_t _thisIndex)
	: textureFilePath_(_texFilePath), thisIndex_(_thisIndex) {

	/// sprite
	uint32_t texHandle = TextureManager::GetInstance()->LoadTexture(textureFilePath_);
	sprite_.reset(Sprite::Create(
		texHandle,
		{ 0.0f, 0.0f },
		{ 1.0f, 1.0f, 1.0f, 1.0f }
	));

	sprite_->anchorPoint_ = { 0.5f, 0.5f };

	maxScale_ = 0.5f;
	minScale_ = 0.3f;
	scale_ = minScale_;
	sprite_->SetScale({ scale_, scale_ });
}

void PauseMenuItem::Update(size_t _currentIndex) {
	/// 選択されているか
	if (_currentIndex == thisIndex_) {
		isSelected_ = true;
	} else {
		isSelected_ = false;
	}

	/// 拡大率
	if (isSelected_) {
		scale_ += (maxScale_ - scale_) * 0.1f;
		colorScale_ += (1.0f - colorScale_) * 0.1f;
	} else {
		scale_ += (minScale_ - scale_) * 0.1f;
		colorScale_ += (0.5f - colorScale_) * 0.1f;
	}

	sprite_->SetScale({ scale_, scale_ });
	sprite_->SetColor(baseColor_ * colorScale_);
}

void PauseMenuItem::Draw() {
	sprite_->Draw();
}


bool PauseMenuItem::GetIsSelected() const {
	return isSelected_;
}

bool PauseMenuItem::GetIsActive() const {
	return isActive_;
}

float PauseMenuItem::GetScale() const {
	return scale_;
}

void PauseMenuItem::SetIsSelected(bool _isSelected) {
	isSelected_ = _isSelected;
}

void PauseMenuItem::SetIsActive(bool _isActive) {
	isActive_ = _isActive;
}

void PauseMenuItem::SetScale(float _scale) {
	scale_ = _scale;
}

void PauseMenuItem::SetRotation(float _rotation) {
	rotation_ = _rotation;
}

void PauseMenuItem::SetPosition(const Vector2& _position) {
	sprite_->SetPosition(_position);
}