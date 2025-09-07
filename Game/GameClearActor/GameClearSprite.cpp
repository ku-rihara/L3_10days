#include "GameClearSprite.h"

/// engine
#include "base/TextureManager.h"

GameClearSprite::GameClearSprite() = default;
GameClearSprite::~GameClearSprite() = default;

void GameClearSprite::Init() {
	uint32_t textureHandle = TextureManager::GetInstance()->LoadTexture(
		"./resources/Texture/GameResult/GameClear.png");
	sprite_.reset(Sprite::Create(textureHandle, Vector2(0, 0), { 1, 1, 1, 1 }));
	/// アンカーポイント
	sprite_->anchorPoint_ = { 0.5f, 0.5f };
	/// 画面中央に表示
	Vector2 position = { 640.0f, 360.0f };
	sprite_->SetPosition(position);
	/// スプライトのサイズを取得、スケールの調整
	Vector2 spriteSize = sprite_->GetTextureSize();
	Vector2 renderSize = { 1280.0f, 720.0f };
	sprite_->SetScale(renderSize / spriteSize);
}

void GameClearSprite::Update() {}

void GameClearSprite::Draw() {
	sprite_->Draw();
}

