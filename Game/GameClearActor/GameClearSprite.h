#pragma once

/// std
#include <memory>

/// engine
#include "2d/Sprite.h"

/// game
#include "Actor/UI/NumDraw/NumDraw.h"

class GameClearSprite {

	enum {
		RESULT_WINDOW,		//< ウィンドウ
		MISSION_RESULT,		//< ミッションリザルト
		TOTAL_TIME,			//< 経過時間
		TOTAL_TIME_COMMA_1,	//< 経過時間
		TOTAL_TIME_COMMA_2,	//< 経過時間
		TIME_BONUS,			//< タイムボーナス
		DESTROY_COUNT,		//< 撃破機体数
		DESTROY_COUNT_UNIT, //< 撃破機体数
		DESTROY_BONUS,		//< 撃破ボーナス
		FINAL_SCORE,		//< 最終スコア
		LINE,				//< 境界線
		TO_TITLE,			//< タイトルに戻る
		MAX_SPRITE
	};

	enum {
		TOTAL_TIME_MIN,		//< 経過時間(分)
		TOTAL_TIME_SEC,		//< 経過時間(秒)
		TOTAL_TIME_FRAC,	//< 経過時間(小数点以下2桁)
		TIME_BONUS_NUM,		//< タイムボーナス
		DESTROY_COUNT_NUM,	//< 撃破機体数
		DESTROY_BONUS_NUM,	//< 撃破ボーナス
		FINAL_SCORE_NUM,	//< 最終スコア
		MAX_NUM_DRAW
	};

public:
	GameClearSprite();
	~GameClearSprite();

	void Init();
	void Update();
	void Draw();

	void InitSprites();
	void InitNumDraws();

	void SetTotalTime(float _time);
	int32_t SetTimeBonus(float _time);
	int32_t SetDestroyBonus(int32_t _count);
	void SetTotalScore(int32_t _score);

private:
	using USprite = std::unique_ptr<Sprite>;
	using UNumDraw = std::unique_ptr<NumDraw>;

	std::vector<USprite> sprites_;

	/// total time
	std::vector<UNumDraw> numDraws_;

};

