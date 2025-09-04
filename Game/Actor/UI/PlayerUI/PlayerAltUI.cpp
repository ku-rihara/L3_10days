#include "PlayerAltUI.h"

#include "base/TextureManager.h"

#include "Actor/Player/Player.h"

void PlayerAltUI::Init() {
	altUISprite_ = std::make_unique<Sprite>();
	altUISprite_->CreateSprite(TextureManager::GetInstance()->LoadTexture("./resources/Texture/UI/Alt.png"), { 100,100 }, { 1,1,1,1 });
	altUISprite_->anchorPoint_ = { 0.5f,0.5f };
	
	Vector2 position = { (1280.0f / 2.0f) + (1280.0f / 2.0f) / 3.0f, 360.0f };
	altUISprite_->SetPosition(position);
	
	Vector2 size = { 64, 64 };
	Vector2 texSize = altUISprite_->GetTextureSize();
	altUISprite_->SetScale({ size.x / texSize.x, size.y / texSize.y });


	altNumDraw_ = std::make_unique<NumDraw>();
	altNumDraw_->Init(4);
	altNumDraw_->SetBasePosition({ position.x, position.y + 2.0f });
	altNumDraw_->SetDigitSpacing(10.0f);
	altNumDraw_->SetNumber(1964);

}

void PlayerAltUI::Update(Player* _player) {
	altNumDraw_->Update();

	altNumDraw_->SetNumber(static_cast<int32_t>(_player->GetPosition().y));
}

void PlayerAltUI::Draw() {
	altUISprite_->Draw();
	altNumDraw_->Draw();
}