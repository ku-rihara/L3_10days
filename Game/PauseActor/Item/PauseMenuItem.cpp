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
		scale_ += (1.2f - scale_) * 0.1f;
		alpha_ += (1.0f - alpha_) * 0.1f;
	} else {
		scale_ += (1.0f - scale_) * 0.1f;
		alpha_ += (0.5f - alpha_) * 0.1f;
	}

	sprite_->SetScale({ scale_, scale_ });
	sprite_->SetAlpha(alpha_);
}

void PauseMenuItem::Draw() {}


bool PauseMenuItem::GetIsSelected() const {
	return isSelected_;
}

bool PauseMenuItem::GetIsActive() const {
	return isActive_;
}

float PauseMenuItem::GetScale() const {
	return scale_;
}

float PauseMenuItem::GetAlpha() const {
	return alpha_;
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

void PauseMenuItem::SetAlpha(float _alpha) {
	alpha_ = _alpha;
}

void PauseMenuItem::SetPosition(const Vector2& _position) {
	sprite_->SetPosition(_position);
}