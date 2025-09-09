#pragma once

#include <memory>
#include "2d/Sprite.h"

/// ///////////////////////////////////////////////////////////////
/// Fade
/// ///////////////////////////////////////////////////////////////
class Fade {
public:
	/// ==================================
	/// public : methods
	/// ==================================
	Fade() = default;
	~Fade() = default;
	void Init();
	void Update();
	void Draw();
	/// フェードイン
	void FadeIn(float speed = 0.02f);
	/// フェードアウト
	void FadeOut(float speed = 0.02f);
	/// フェード状態の取得
	bool IsFade() { return isFade_; }
	/// フェードイン中か
	bool IsFadeIn() { return isFadeIn_; }
	/// フェードアウト中か
	bool IsFadeOut() { return isFadeOut_; }
	/// フェード完了か
	bool IsFadeEnd() { return isFadeEnd_; }
	/// 透明度の取得
	float GetAlpha() { return alpha_; }

private:
	/// ==================================
	/// private : variables
	/// ==================================

	std::unique_ptr<Sprite> sprite_ = nullptr; /// スプライト

	float speed_ = 0.02f; /// フェード速度
	float alpha_ = 0.0f; /// 透明度
	bool isFade_ = false; /// フェード中か
	bool isFadeIn_ = false; /// フェードイン中か
	bool isFadeOut_ = false; /// フェードアウト中か
	bool isFadeEnd_ = false; /// フェード完了か

};

