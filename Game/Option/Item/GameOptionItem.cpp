#include "GameOptionItem.h"

#include "base/TextureManager.h"

void IGameOptionItem::BaseInit(const std::string& _texFilepath, size_t _thisIndex) {
	thisIndex_ = _thisIndex;

	uint32_t texHandle = TextureManager::GetInstance()->LoadTexture(_texFilepath);
	sprite_.reset(Sprite::Create(
		texHandle,
		{ 0.0f, 0.0f },
		{ 1.0f, 1.0f, 1.0f, 1.0f }
	));
	
	sprite_->anchorPoint_ = { 0.5f, 0.5f };

}

