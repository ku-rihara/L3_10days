#include "NumDraw.h"

/// engine
#include "base/TextureManager.h"

void NumDraw::Init(size_t _maxNumDigit) {
	maxNumDigit_ = _maxNumDigit;
	basePosition_ = { 1280.0f / 2.0f, 100.0f };
	for (size_t i = 0; i < maxNumDigit_; ++i) {
		numSprites_.emplace_back(std::make_unique<Sprite>());
		numSprites_[i]->CreateSprite(TextureManager::GetInstance()->LoadTexture("./resources/Texture/UI/Num.png"), { 100, 100 }, { 1, 1, 1, 1 });
		numSprites_[i]->anchorPoint_ = { 0.5f, 0.5f };
		size_ = Vector2{ 4.0f, 5.0f } *2.0f;
		texSize_ = numSprites_[i]->GetTextureSize();
		numSprites_[i]->SetScale({ size_.x / texSize_.x, size_.y / texSize_.y });
		numSprites_[i]->SetColor({ 0.239f, 1.0f, 0.239f });
	}

	isDrawAll_ = false;
}

void NumDraw::Update() {
	if (previousNum_ != currentNum_) {
		previousNum_ = currentNum_;
		digitNum_ = GetDigitNum(currentNum_);
		if (digitNum_ > static_cast<int32_t>(maxNumDigit_)) {
			digitNum_ = static_cast<int32_t>(maxNumDigit_);
		}
		int32_t num = currentNum_;

		// 桁を一時的に保存
		std::vector<int32_t> digits(digitNum_);
		for (int32_t i = 0; i < digitNum_; ++i) {
			digits[digitNum_ - 1 - i] = num % 10; // ←逆順に格納
			num /= 10;
		}


		if (isDrawAll_) {
			// --- 全桁表示（ゼロ埋めあり）
			for (size_t i = 0; i < maxNumDigit_; ++i) {
				int32_t digitIndex = static_cast<int32_t>(i - (maxNumDigit_ - digitNum_));

				int32_t digit = 0;
				if (digitIndex >= 0) {
					// digits[] から値を取る（有効桁）
					digit = digits[digitIndex];
				}
				// 桁をセット
				numSprites_[i]->uvTransform_.pos = { digit / 10.0f, 0.0f };
				numSprites_[i]->uvTransform_.scale = { 1.0f / 10.0f, 1.0f };

				// 位置は maxNumDigit_ を基準に中央揃え
				numSprites_[i]->SetPosition({
					basePosition_.x - digitSpacing_ * (maxNumDigit_ - 1) / 2.0f + digitSpacing_ * i,
					basePosition_.y
					});
				numSprites_[i]->uvTransform_.rotate = { 0.0f, 0.0f, 0.0f };
			}
		} else {
			// --- 有効桁だけ表示（従来処理）
			for (size_t i = 0; i < (maxNumDigit_); ++i) {
				if (i < digitNum_) {
					int32_t digit = digits[i];
					numSprites_[i]->uvTransform_.pos = { digit / 10.0f, 0.0f };
					numSprites_[i]->uvTransform_.scale = { 1.0f / 10.0f, 1.0f };
					numSprites_[i]->SetPosition({
						basePosition_.x - digitSpacing_ * (digitNum_ - 1) / 2.0f + digitSpacing_ * i,
						basePosition_.y
						});
					numSprites_[i]->uvTransform_.rotate = { 0.0f, 0.0f, 0.0f };
				} else {
					numSprites_[i]->uvTransform_.pos = { 1.0f, 0.0f };
					numSprites_[i]->uvTransform_.scale = { 0.0f, 0.0f };
				}
			}
		}
	}
}


void NumDraw::Draw() {
	for (size_t i = 0; i < maxNumDigit_; ++i) {
		numSprites_[i]->Draw();
	}
}

void NumDraw::SetNumber(int32_t _num) {
	currentNum_ = _num;
}

int32_t NumDraw::GetDigitNum(int32_t _num) {
	if (_num == 0) {
		return 1;
	}
	int32_t num = _num;
	int32_t digit = 0;
	while (num > 0) {
		num /= 10;
		digit++;
	}
	return digit;
}

void NumDraw::SetBasePosition(const Vector2& _pos) {
	basePosition_ = _pos;
}

void NumDraw::SetDigitSpacing(float _spacing) {
	digitSpacing_ = _spacing;
}

void NumDraw::SetMaxNumDigit(size_t _maxNumDigit) {
	maxNumDigit_ = _maxNumDigit;
}

void NumDraw::SetColor(const Vector4& _color) {
	for (size_t i = 0; i < maxNumDigit_; ++i) {
		numSprites_[i]->SetColor({ _color.x, _color.y, _color.z });
		numSprites_[i]->SetAlpha(_color.w);
	}
}

void NumDraw::SetScale(const Vector2& _scale) {
	Vector2 scale = Vector2{ size_.x / texSize_.x, size_.y / texSize_.y } *_scale;
	for (size_t i = 0; i < maxNumDigit_; ++i) {
		numSprites_[i]->SetScale(scale);
	}
}

void NumDraw::SetIsDrawAll(bool _isDrawAll) {
	isDrawAll_ = _isDrawAll;
}
