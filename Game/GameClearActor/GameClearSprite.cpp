#include "GameClearSprite.h"

/// engine
#include "base/TextureManager.h"

GameClearSprite::GameClearSprite() = default;
GameClearSprite::~GameClearSprite() = default;


void GameClearSprite::Init() {

	sprites_.resize(MAX_SPRITE);

	float centerX = 640.0f - 200.0f;

	{   /// RESULT_WINDOW
		auto& sp = sprites_[RESULT_WINDOW];
		uint32_t texHandle = TextureManager::GetInstance()->LoadTexture("./resources/Texture/GameResult/ResultWindow.png");
		sp.reset(Sprite::Create(texHandle, { centerX, 360.0f }, { 1, 1, 1, 1 }));
		sp->anchorPoint_ = { 0.5f, 0.5f };
	}

	{   /// MISSION_RESULT
		auto& sp = sprites_[MISSION_RESULT];
		uint32_t texHandle = TextureManager::GetInstance()->LoadTexture("./resources/Texture/GameResult/MissionResult.png");
		sp.reset(Sprite::Create(texHandle, { centerX, 120.0f }, { 1, 1, 1, 1 }));
		sp->anchorPoint_ = { 0.5f, 0.5f };
	}



	{   /// TOTAL_TIME
		auto& sp = sprites_[TOTAL_TIME];
		uint32_t texHandle = TextureManager::GetInstance()->LoadTexture("./resources/Texture/GameResult/TotalTime.png");
		sp.reset(Sprite::Create(texHandle, { 160.0f, 220.0f }, { 1, 1, 1, 1 }));
		sp->anchorPoint_ = { 0.0f, 0.5f };
	}

	{   /// TIME_BONUS
		auto& sp = sprites_[TIME_BONUS];
		uint32_t texHandle = TextureManager::GetInstance()->LoadTexture("./resources/Texture/GameResult/TimeBonus.png");
		sp.reset(Sprite::Create(texHandle, { 160.0f, 280.0f }, { 1, 1, 1, 1 }));
		sp->anchorPoint_ = { 0.0f, 0.5f };
	}

	{   /// DESTROY_COUNT
		auto& sp = sprites_[DESTROY_COUNT];
		uint32_t texHandle = TextureManager::GetInstance()->LoadTexture("./resources/Texture/GameResult/TotalBreak.png");
		sp.reset(Sprite::Create(texHandle, { 160.0f, 340.0f }, { 1, 1, 1, 1 }));
		sp->anchorPoint_ = { 0.0f, 0.5f };
	}

	{   /// DESTROY_BONUS
		auto& sp = sprites_[DESTROY_BONUS];
		uint32_t texHandle = TextureManager::GetInstance()->LoadTexture("./resources/Texture/GameResult/BreakBonus.png");
		sp.reset(Sprite::Create(texHandle, { 160.0f, 400.0f }, { 1, 1, 1, 1 }));
		sp->anchorPoint_ = { 0.0f, 0.5f };
	}

	{   /// LINE
		auto& sp = sprites_[LINE];
		uint32_t texHandle = TextureManager::GetInstance()->LoadTexture("./resources/Texture/GameResult/Line.png");
		sp.reset(Sprite::Create(texHandle, { centerX, 460.0f }, { 1, 1, 1, 1 }));
		sp->anchorPoint_ = { 0.5f, 0.5f };
	}

	{   /// FINAL_SCORE
		auto& sp = sprites_[FINAL_SCORE];
		uint32_t texHandle = TextureManager::GetInstance()->LoadTexture("./resources/Texture/GameResult/TotalScore.png");
		sp.reset(Sprite::Create(texHandle, { 160.0f, 520.0f }, { 1, 1, 1, 1 }));
		sp->anchorPoint_ = { 0.0f, 0.5f };
	}


	{   /// TO_TITLE
		auto& sp = sprites_[TO_TITLE];
		uint32_t texHandle = TextureManager::GetInstance()->LoadTexture("./resources/Texture/GameResult/ReturnTitle.png");
		sp.reset(Sprite::Create(texHandle, { 980.0f, 600.0f }, { 1, 1, 1, 1 })); // ←右寄せ
		sp->anchorPoint_ = { 0.5f, 0.5f };
		Vector2 texSize = sp->GetTextureSize();
		Vector2 size = { 300.0f, 50.0f };
		sp->SetScale(size / texSize);
	}
}



void GameClearSprite::Update() {}

void GameClearSprite::Draw() {
	for (auto& sp : sprites_) {
		sp->Draw();
	}
}

