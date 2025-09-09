#include "GameTimer.h"

/// engine
#include "base/TextureManager.h"
#include "Frame/Frame.h"

GameTimer::GameTimer() = default;
GameTimer::~GameTimer() = default;

void GameTimer::Init() {
	numDraws_.resize(3);
	for (int i = 0; i < numDraws_.size(); i++) {
		numDraws_[i] = std::make_unique<NumDraw>();
		numDraws_[i]->Init(2);
		numDraws_[i]->SetIsDrawAll(true);
		numDraws_[i]->SetAlignment(NumDraw::Alignment::Left);
		numDraws_[i]->SetColor({ 1.0f, 1.0f, 1.0f, 1.0f });
		numDraws_[i]->SetDigitSpacing(28.0f);
		numDraws_[i]->SetBaseSize({ 32.0f, 32.0f });
	}

	/// 各桁の位置調整、ウィンドウの左上に表示
	Vector2 startPos = { 32, 32 };
	Vector2 offset = { 72, 0 };
	for (size_t i = 0; i < numDraws_.size(); i++) {
		auto& nd = numDraws_[i];
		nd->SetBasePosition(startPos + offset * static_cast<float>(i));
	}

	/// ":"の表示
	colons_.resize(2);
	int32_t texHandle = TextureManager::GetInstance()->LoadTexture("./resources/Texture/UI/colon.png");
	for (auto& colon : colons_) {
		colon.reset(Sprite::Create(texHandle, { 0, 0 }, { 1, 1, 1, 1 }));
		colon->anchorPoint_ = { 0.5f, 0.5f };
		colon->SetScale({ 0.5f, 0.5f });
	}

	/// ":"の位置調整
	colons_[0]->SetPosition(startPos + Vector2(52, 0));
	colons_[1]->SetPosition(startPos + Vector2(124, 0));


	time_ = 0.0f;
}

void GameTimer::Update(bool _isGameClear) {

	/// ゲームクリアしていなければ時間を進める
	if (!_isGameClear) {
		time_ += Frame::DeltaTime();
	}

	int intTime = static_cast<int>(time_);
	if (intTime < 0) {
		intTime = 0;
	}

	/// 分と秒、少数部に変換
	int32_t minutes = intTime / 60;
	int32_t seconds = intTime % 60;
	int32_t fraction = static_cast<int32_t>((time_ - intTime) * 100);

	int32_t nums[] = { minutes, seconds, fraction };
	for (int i = 0; i < numDraws_.size(); i++) {
		numDraws_[i]->SetNumber(nums[i]);
		numDraws_[i]->Update();
	}

}

void GameTimer::Draw() {
	for (auto& numDraw : numDraws_) {
		numDraw->Draw();
	}

	for (auto& sp : colons_) {
		sp->Draw();
	}
}

float GameTimer::GetTime() const {
	return time_;
}