#include "GameClearSprite.h"

/// engine
#include "base/TextureManager.h"

GameClearSprite::GameClearSprite() = default;
GameClearSprite::~GameClearSprite() = default;

void GameClearSprite::Init() {

	sprites_.resize(MAX_SPRITE);

	{	/// RESULT_WINDOW
		auto& sp = sprites_[RESULT_WINDOW];
		uint32_t textureHandle = TextureManager::GetInstance()->LoadTexture("./resources/Texture/GameResult/ResultWindow.png");
		sp.reset(Sprite::Create(textureHandle, { 640.0f, 360.0f }, { 1, 1, 1, 1 }));
		sp->anchorPoint_ = { 0.5f, 0.5f };
		Vector2 texSize = sp->GetTextureSize();
		Vector2 size = Vector2{ 800.0f, 600.0f };
		sp->SetScale(size / texSize);
	}

	{	/// MISSION_RESULT
		auto& sp = sprites_[MISSION_RESULT];
		uint32_t textureHandle = TextureManager::GetInstance()->LoadTexture("./resources/Texture/GameResult/MissionResult.png");
		sp.reset(Sprite::Create(textureHandle, { 640.0f, 100.0f }, { 1, 1, 1, 1 }));
		sp->anchorPoint_ = { 0.5f, 0.5f };
		Vector2 texSize = sp->GetTextureSize();
		Vector2 size = Vector2{ 400.0f, 100.0f };
		sp->SetScale(size / texSize);
	}

	{	/// TOTAL_TIME
		auto& sp = sprites_[TOTAL_TIME];
		uint32_t textureHandle = TextureManager::GetInstance()->LoadTexture("./resources/Texture/GameResult/TotalTime.png");
		sp.reset(Sprite::Create(textureHandle, { 400.0f, 200.0f }, { 1, 1, 1, 1 }));
		sp->anchorPoint_ = { 0.0f, 0.5f };
		Vector2 texSize = sp->GetTextureSize();
		Vector2 size = Vector2{ 200.0f, 50.0f };
		sp->SetScale(size / texSize);
	}

	{	/// TIME_BONUS
		auto& sp = sprites_[TIME_BONUS];
		uint32_t textureHandle = TextureManager::GetInstance()->LoadTexture("./resources/Texture/GameResult/TimeBonus.png");
		sp.reset(Sprite::Create(textureHandle, { 400.0f, 260.0f }, { 1, 1, 1, 1 }));
		sp->anchorPoint_ = { 0.0f, 0.5f };
		Vector2 texSize = sp->GetTextureSize();
		Vector2 size = Vector2{ 200.0f, 50.0f };
		sp->SetScale(size / texSize);
	}

	{	/// DESTROY_COUNT
		auto& sp = sprites_[DESTROY_COUNT];
		uint32_t textureHandle = TextureManager::GetInstance()->LoadTexture("./resources/Texture/GameResult/TotalBreak.png");
		sp.reset(Sprite::Create(textureHandle, { 400.0f, 320.0f }, { 1, 1, 1, 1 }));
		sp->anchorPoint_ = { 0.0f, 0.5f };
		Vector2 texSize = sp->GetTextureSize();
		Vector2 size = Vector2{ 200.0f, 50.0f };
		sp->SetScale(size / texSize);
	}

	{	/// DESTROY_BONUS
		auto& sp = sprites_[DESTROY_BONUS];
		uint32_t textureHandle = TextureManager::GetInstance()->LoadTexture("./resources/Texture/GameResult/BreakBonus.png");
		sp.reset(Sprite::Create(textureHandle, { 400.0f, 380.0f }, { 1, 1, 1, 1 }));
		sp->anchorPoint_ = { 0.0f, 0.5f };
		Vector2 texSize = sp->GetTextureSize();
		Vector2 size = Vector2{ 200.0f, 50.0f };
		sp->SetScale(size / texSize);
	}

	{	/// FINAL_SCORE
		auto& sp = sprites_[FINAL_SCORE];
		uint32_t textureHandle = TextureManager::GetInstance()->LoadTexture("./resources/Texture/GameResult/TotalScore.png");
		sp.reset(Sprite::Create(textureHandle, { 400.0f, 440.0f }, { 1, 1, 1, 1 }));
		sp->anchorPoint_ = { 0.0f, 0.5f };
		Vector2 texSize = sp->GetTextureSize();
		Vector2 size = Vector2{ 200.0f, 50.0f };
		sp->SetScale(size / texSize);
	}

	{	/// LINE
		auto& sp = sprites_[LINE];
		uint32_t textureHandle = TextureManager::GetInstance()->LoadTexture("./resources/Texture/GameResult/Line.png");
		sp.reset(Sprite::Create(textureHandle, { 640.0f, 500.0f }, { 1, 1, 1, 1 }));
		sp->anchorPoint_ = { 0.5f, 0.5f };
		Vector2 texSize = sp->GetTextureSize();
		Vector2 size = Vector2{ 600.0f, 5.0f };
		sp->SetScale(size / texSize);
	}

	{	/// TO_TITLE
		auto& sp = sprites_[TO_TITLE];
		uint32_t textureHandle = TextureManager::GetInstance()->LoadTexture("./resources/Texture/GameResult/ReturnTitle.png");
		sp.reset(Sprite::Create(textureHandle, { 640.0f, 550.0f }, { 1, 1, 1, 1 }));
		sp->anchorPoint_ = { 0.5f, 0.5f };
		Vector2 texSize = sp->GetTextureSize();
		Vector2 size = Vector2{ 300.0f, 50.0f };
		sp->SetScale(size / texSize);
	}

}

void GameClearSprite::Update() {}

void GameClearSprite::Draw() {
	for (auto& sp : sprites_) {
		sp->Draw();
	}
}

