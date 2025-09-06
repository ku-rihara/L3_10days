#include "OperationOptionItem.h"

/// engine
#include "base/TextureManager.h"

void OperationOptionItem::Init() {

	uint32_t texHandle = TextureManager::GetInstance()->LoadTexture(
		"./resources/Texture/Option/Controller.png"
	);

	controllerSprite_.reset(Sprite::Create(
		texHandle,
		{ 900.0f, 360.0f }, { 1, 1, 1, 1.0f })
	);

	controllerSprite_->anchorPoint_ = { 0.5f, 0.5f };
	controllerSprite_->SetScale(Vector2(1, 1) / 3.0f);

}

void OperationOptionItem::Update(size_t _currentIndex) {
	if (thisIndex_ != _currentIndex) {
		return;
	}
}

void OperationOptionItem::Draw() {

	controllerSprite_->Draw();

}
