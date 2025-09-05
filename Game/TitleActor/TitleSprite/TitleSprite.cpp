#include "TitleSprite.h"

#include "base/TextureManager.h"

void TitleSprite::Init() {

	uint32_t texHandle = TextureManager::GetInstance()->LoadTexture("./resources/Texture/Title/title.png");

	Vector2 winSize = { 1280.0f, 720.0f };
	sprite_.reset(Sprite::Create(texHandle, winSize / 2.0f, { 1, 1, 1, 1 }));
	sprite_->anchorPoint_ = { 0.5f, 0.5f };
}

void TitleSprite::Update() {

}

void TitleSprite::Draw() {
	sprite_->Draw();
}