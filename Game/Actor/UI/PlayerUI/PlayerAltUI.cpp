#include "PlayerAltUI.h"

#include "base/TextureManager.h"

#include "Actor/Player/Player.h"

void PlayerAltUI::Init() {
	altUISprite_ = std::make_unique<Sprite>();
	altUISprite_->CreateSprite(TextureManager::GetInstance()->LoadTexture("./resources/Texture/UI/Alt.png"), { 100, 100 }, { 1, 1, 1, 1 });
	altUISprite_->anchorPoint_ = { 0.5f, 0.5f };

	Vector2 position = { (1280.0f / 2.0f) + (1280.0f / 2.0f) / 3.0f, 360.0f };
	altUISprite_->SetPosition(position);

	Vector2 size = { 64, 64 };
	Vector2 texSize = altUISprite_->GetTextureSize();
	altUISprite_->SetScale({ size.x / texSize.x, size.y / texSize.y });



	int32_t textureHandle = TextureManager::GetInstance()->LoadTexture(
		"./resources/Texture/UI/Minus.png");
	minusSprite_.reset(Sprite::Create(
		textureHandle, { position.x, position.y + 2.0f }, { 1, 1, 1, 1 }));
	minusSprite_->anchorPoint_ = { 0.5f, 0.5f };
	Vector2 minusSize = { 32, 32 };
	Vector2 minusTexSize = minusSprite_->GetTextureSize();
	minusSprite_->SetScale({ minusSize.x / minusTexSize.x, minusSize.y / minusTexSize.y });
	minusSprite_->SetColor({ 0.239f, 1.0f, 0.239f });


	altNumDraw_ = std::make_unique<NumDraw>();
	altNumDraw_->Init(4);
	altNumDraw_->SetBasePosition({ position.x + 16.0f, position.y + 2.0f });
	altNumDraw_->SetDigitSpacing(10.0f);
	altNumDraw_->SetAlignment(NumDraw::Alignment::Right);

}

void PlayerAltUI::Update(Player* _player) {
	altNumDraw_->Update();

	int32_t alt = static_cast<int32_t>(_player->GetPosition().y);
	isMinus_ = IsMinus(alt);
	if (isMinus_) {
		alt *= -1;
	}

	altNumDraw_->SetNumber(alt);

	minusSprite_->SetPosition({
		altNumDraw_->GetBasePosition().x - (altNumDraw_->GetDigitSpacing() * altNumDraw_->GetNumDigits()) ,
		minusSprite_->GetPosition().y
		});

}

void PlayerAltUI::Draw() {
	altUISprite_->Draw();
	altNumDraw_->Draw();

	if (isMinus_) {
		minusSprite_->Draw();
	}
}

bool PlayerAltUI::IsMinus(int32_t _value) {
	if (_value < 0) {
		return true;
	}

	return false;
}
