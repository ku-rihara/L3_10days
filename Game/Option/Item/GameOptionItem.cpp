#include "GameOptionItem.h"

#include "base/TextureManager.h"

void IGameOptionItem::BaseInit(const std::string& _texFilepath, size_t _thisIndex) {
	thisIndex_ = _thisIndex;

	uint32_t texHandle = TextureManager::GetInstance()->LoadTexture(_texFilepath);
	textSprite_.reset(Sprite::Create(
		texHandle, { 0.0f, 0.0f },
		{ 1.0f, 1.0f, 1.0f, 1.0f }
	));

	textSprite_->anchorPoint_ = { 0.5f, 0.5f };

	/// 初期位置
	Vector2 pos = startPos_ + offsetPos_ * static_cast<float>(thisIndex_);
	textSprite_->SetPosition(pos);

	/// 拡縮率
	Vector2 texSize = textSprite_->GetTextureSize();
	Vector2 renderingSize = Vector2{ 400.0f, 100.0f } / 3.0f;
	textSprite_->SetScale(renderingSize / texSize);

}

void IGameOptionItem::BaseUpdate(size_t _currentIndex) {

	/// 選択されているか
	isSelected_ = (thisIndex_ == _currentIndex);

	/// 選択されているなら拡大
	if (isSelected_) {
		scale_ += (1.2f - scale_) * 0.1f;
		rotation_ += (0.05f - rotation_) * 0.1f;
		colorScale_ += (1.0f - colorScale_) * 0.1f;
	} else {
		scale_ += (1.0f - scale_) * 0.1f;
		rotation_ += (0.0f - rotation_) * 0.1f;
		colorScale_ += (0.5f - colorScale_) * 0.1f;
	}

	textSprite_->SetColor({ colorScale_, colorScale_, colorScale_ });

}

void IGameOptionItem::BaseDraw() {
	textSprite_->Draw();
}

const Vector2& IGameOptionItem::GetStartPos() const {
	return startPos_;
}

const Vector2& IGameOptionItem::GetOffsetPos() const {
	return offsetPos_;
}

