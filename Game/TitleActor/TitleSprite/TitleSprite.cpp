#include "TitleSprite.h"

#include "base/TextureManager.h"

void TitleSprite::Init() {

	//uint32_t texHandle = TextureManager::GetInstance()->LoadTexture("./resources/Texture/Title/title.png");

}

void TitleSprite::Update() {

}

void TitleSprite::Draw() {
	sprite_->Draw();
}