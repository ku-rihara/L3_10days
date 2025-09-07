#include "GameOverSprite.h"

/// engine
#include "base/TextureManager.h"

GameOverSprite::GameOverSprite() = default;
GameOverSprite::~GameOverSprite() = default;

void GameOverSprite::Init() {
	uint32_t textureHandle = TextureManager::GetInstance()->LoadTexture(
		"./resources/Texture/GameOver/GameOver.png");
	sprite_.reset(Sprite::Create(textureHandle, Vector2(0, 0), { 1, 1, 1, 1 }));

	/// アンカーポイント
	sprite_->anchorPoint_ = { 0.5f, 0.5f };

	/// 画面中央に表示
	Vector2 position = { 640.0f, 360.0f };
	sprite_->SetPosition(position);

	/// スプライトのサイズを取得、スケールの調整
	Vector2 spriteSize = sprite_->GetTextureSize();
	Vector2 renderSize = { 800.0f, 450.0f };
	sprite_->SetScale(renderSize / spriteSize);

}

void GameOverSprite::Update() {}


void GameOverSprite::Draw() {
	sprite_->Draw();
}
