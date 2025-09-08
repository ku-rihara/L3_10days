#pragma once

/// std
#include <memory>

/// engine
#include "2d/Sprite.h"

class GameClearSprite {

	enum {
		RESULT_WINDOW,  //< ウィンドウ
		MISSION_RESULT, //< ミッションリザルト
		TOTAL_TIME,	    //< 経過時間
		TIME_BONUS,	    //< タイムボーナス
		DESTROY_COUNT,  //< 撃破機体数
		DESTROY_BONUS,  //< 撃破ボーナス
		FINAL_SCORE,    //< 最終スコア
		LINE,           //< 境界線
		TO_TITLE,       //< タイトルに戻る
		MAX_SPRITE
	};

public:
	GameClearSprite();
	~GameClearSprite();
	void Init();
	void Update();
	void Draw();

private:
	using USprite = std::unique_ptr<Sprite>;
	std::vector<USprite> sprites_;
};

