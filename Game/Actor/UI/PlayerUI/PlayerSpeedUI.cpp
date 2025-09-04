#include "PlayerSpeedUI.h"

/// engine
#include "base/TextureManager.h"

void PlayerSpeedUI::Init() {
	speedUISprite_ = std::make_unique<Sprite>();
	speedUISprite_->CreateSprite(TextureManager::GetInstance()->LoadTexture("./resources/Texture/UI/Speed.png"), { 100,100 }, { 1,1,1,1 });
	speedUISprite_->anchorPoint_ = { 0.5f,0.5f };

	Vector2 position = { (1280.0f / 2.0f) - (1280.0f / 2.0f) / 3.0f, 360.0f };
	speedUISprite_->SetPosition(position);

	Vector2 size = { 64, 64 };
	Vector2 texSize = speedUISprite_->GetTextureSize();
	speedUISprite_->SetScale({ size.x / texSize.x, size.y / texSize.y });
}

void PlayerSpeedUI::Update() {
}

void PlayerSpeedUI::Draw() {
	speedUISprite_->Draw();
}