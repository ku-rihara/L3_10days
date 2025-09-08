#include "GameClearSprite.h"

/// engine
#include "base/TextureManager.h"
#include "Actor/GameController/GameScore.h"

GameClearSprite::GameClearSprite() = default;
GameClearSprite::~GameClearSprite() = default;


void GameClearSprite::Init() {
	InitSprites();
	InitNumDraws();
}



void GameClearSprite::Update() {
	/// 各種数値セット
	GameScore* gameScore = GameScore::GetInstance();
	SetTotalTime(gameScore->GetClearTime());
	int32_t timeBonus = SetTimeBonus(gameScore->GetClearTime());
	int32_t destroyBonus = SetDestroyBonus(gameScore->GetBreakEnemyCount());
	SetTotalScore(destroyBonus + timeBonus);

	for (auto& nd : numDraws_) {
		nd->Update();
	}
}

void GameClearSprite::Draw() {
	for (auto& sp : sprites_) {
		sp->Draw();
	}

	for (auto& nd : numDraws_) {
		nd->Draw();
	}
}

/// //////////////////////////////////////////////////////////////////////////////////////////
/// InitSprites
/// //////////////////////////////////////////////////////////////////////////////////////////
void GameClearSprite::InitSprites() {

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
	{	/// TOTAL_TIME_COMMA_1
		auto& sp = sprites_[TOTAL_TIME_COMMA_1];
		uint32_t texHandle = TextureManager::GetInstance()->LoadTexture("./resources/Texture/GameResult/Comma.png");
		sp.reset(Sprite::Create(texHandle, { 680.0f - 67.5f, 220.0f }, { 1, 1, 1, 1 }));
		sp->anchorPoint_ = { 0.5f, 0.5f };
	}
	{	/// TOTAL_TIME_COMMA_2
		auto& sp = sprites_[TOTAL_TIME_COMMA_2];
		uint32_t texHandle = TextureManager::GetInstance()->LoadTexture("./resources/Texture/GameResult/Comma.png");
		sp.reset(Sprite::Create(texHandle, { 680.0f - 67.5f * 2.0f, 220.0f }, { 1, 1, 1, 1 }));
		sp->anchorPoint_ = { 0.5f, 0.5f };
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

	{	/// DESTROY_COUNT_UNIT
		auto& sp = sprites_[DESTROY_COUNT_UNIT];
		uint32_t texHandle = TextureManager::GetInstance()->LoadTexture("./resources/Texture/GameResult/Unit.png");
		sp.reset(Sprite::Create(texHandle, { 640.0f, 340.0f }, { 1, 1, 1, 1 }));
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
		sp.reset(Sprite::Create(texHandle, { 980.0f, 600.0f }, { 1, 1, 1, 1 }));
		sp->anchorPoint_ = { 0.5f, 0.5f };
		Vector2 texSize = sp->GetTextureSize();
		Vector2 size = { 300.0f, 50.0f };
		sp->SetScale(size / texSize);
	}
}

/// //////////////////////////////////////////////////////////////////////////////////////////
/// InitNumDraws
/// //////////////////////////////////////////////////////////////////////////////////////////
void GameClearSprite::InitNumDraws() {
	Vector2 scale = Vector2{ 12, 12 };

	numDraws_.resize(MAX_NUM_DRAW);
	for (auto& nd : numDraws_) {
		nd = std::make_unique<NumDraw>("./resources/Texture/GameResult/Numbers.png");
		nd->SetBaseSize({ 3.2f, 4.8f });
		nd->SetColor({ 1, 1, 1, 1 });
		nd->SetDigitSpacing(30.0f);
	}

	{	/// total time

		float fracPos = 665.0f - 16.0f;

		/// 分
		auto& min = numDraws_[TOTAL_TIME_MIN];
		min->Init(2);
		min->SetScale(scale);
		min->SetBasePosition({ fracPos - 140.0f, 220.0f });
		min->SetIsDrawAll(true);

		/// 秒
		auto& sec = numDraws_[TOTAL_TIME_SEC];
		sec->Init(2);
		sec->SetScale(scale);
		sec->SetBasePosition({ fracPos - 70.0f, 220.0f });
		sec->SetIsDrawAll(true);

		/// 小数点以下2桁
		auto& frac = numDraws_[TOTAL_TIME_FRAC];
		frac->Init(2);
		frac->SetScale(scale);
		frac->SetBasePosition({ fracPos, 220.0f });
		frac->SetIsDrawAll(true);

	}

	{	/// time bonus
		auto& tb = numDraws_[TIME_BONUS_NUM];
		tb->Init(6);
		tb->SetScale(scale);
		tb->SetBasePosition({ 665.0f, 280.0f });
		tb->SetAlignment(NumDraw::Alignment::Right);
	}

	{	/// destroy count
		auto& dc = numDraws_[DESTROY_COUNT_NUM];
		dc->Init(3);
		dc->SetScale(scale);
		dc->SetBasePosition({ 665.0f - 64.0f, 340.0f });
	}

	{	/// destroy bonus
		auto& db = numDraws_[DESTROY_BONUS_NUM];
		db->Init(7);
		db->SetScale(scale);
		db->SetBasePosition({ 665.0f, 400.0f });
		db->SetAlignment(NumDraw::Alignment::Right);
	}

	{	/// final score
		auto& fs = numDraws_[FINAL_SCORE_NUM];
		fs->Init(6);
		fs->SetScale(scale);
		fs->SetBasePosition({ 665.0f, 520.0f });
		fs->SetIsDrawAll(true);
		fs->SetAlignment(NumDraw::Alignment::Right);
	}


	for (auto& nd : numDraws_) {
		nd->SetColor({ 1, 1, 1, 1 });
	}
}

void GameClearSprite::SetTotalTime(float _time) {
	/// 分
	int min = static_cast<int>(_time) / 60;
	numDraws_[TOTAL_TIME_MIN]->SetNumber(min);
	/// 秒
	int sec = static_cast<int>(_time) % 60;
	numDraws_[TOTAL_TIME_SEC]->SetNumber(sec);
	/// 小数点以下2桁
	int frac = static_cast<int>((_time - static_cast<int>(_time)) * 100.0f);
	numDraws_[TOTAL_TIME_FRAC]->SetNumber(frac);
}

int32_t GameClearSprite::SetTimeBonus(float _time) {
	/// 30000 - (_time * 500)
	int bonus = static_cast<int>(30000.0f - (_time * 500.0f));
	if (bonus < 0) bonus = 0;
	numDraws_[TIME_BONUS_NUM]->SetNumber(bonus);

	return bonus;
}

int32_t GameClearSprite::SetDestroyBonus(int32_t _count) {
	/// _count * 500
	int bonus = _count * 500;
	numDraws_[DESTROY_COUNT_NUM]->SetNumber(_count);
	numDraws_[DESTROY_BONUS_NUM]->SetNumber(bonus);

	return bonus;
}

void GameClearSprite::SetTotalScore(int32_t _score) {
	numDraws_[FINAL_SCORE_NUM]->SetNumber(_score);
}

