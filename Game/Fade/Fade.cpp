#include "Fade.h"

#include "base/TextureManager.h"
#include "Frame/Frame.h"

void Fade::Init() {
	uint32_t texHandle = TextureManager::GetInstance()->LoadTexture("./resources/Texture/default.png");
	Vector2 winSize = { 1280.0f, 720.0f };

	float color = 0.15f;
	sprite_.reset(Sprite::Create(texHandle, winSize / 2.0f, { color, color, color, 1 }));
	sprite_->anchorPoint_ = { 0.5f, 0.5f };
	sprite_->SetScale(winSize);

	speed_ = 0.0f;
	sprite_->SetAlpha(0.0f);
}

void Fade::Update() {
	if (isFade_) {
		if (isFadeIn_) {
			alpha_ -= speed_ * Frame::DeltaTime();
			if (alpha_ <= 0.0f) {
				alpha_ = 0.0f;
				isFade_ = false;
				isFadeIn_ = false;
				isFadeEnd_ = true;
			}
		}
		if (isFadeOut_) {
			alpha_ += speed_ * Frame::DeltaTime();
			if (alpha_ >= 1.0f) {
				alpha_ = 1.0f;
				isFade_ = false;
				isFadeOut_ = false;
				isFadeEnd_ = true;
			}
		}
	}

	sprite_->SetAlpha(alpha_);
}

void Fade::Draw() {
	sprite_->Draw();
}

void Fade::FadeIn(float speed) {
	if (!isFade_) {
		speed_ = speed;
		isFade_ = true;
		isFadeIn_ = true;
		isFadeOut_ = false;
		isFadeEnd_ = false;
		alpha_ = 1.0f;
	}
}

void Fade::FadeOut(float speed) {
	if (!isFade_) {
		speed_ = speed;
		isFade_ = true;
		isFadeIn_ = false;
		isFadeOut_ = true;
		isFadeEnd_ = false;
		alpha_ = 0.0f;
	}
}

