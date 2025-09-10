#pragma once

#include <memory>

#include "2d/Sprite.h"


class NumDraw {
public:
	enum class Alignment {
		Left,
		Center,
		Right
	};

public:
	NumDraw(const std::string& _filePath = "./resources/Texture/UI/Num.png");
	~NumDraw() = default;

	void Init(size_t _maxNumDigit);
	void Update();
	void Draw();

	void SetNumber(int32_t _num);
	int32_t GetDigitNum(int32_t _num);

	void SetBasePosition(const Vector2& _pos);
	void SetBaseSize(const Vector2& _size);
	void SetDigitSpacing(float _spacing);
	void SetMaxNumDigit(size_t _maxNumDigit);

	void SetColor(const Vector4& _color);
	void SetScale(const Vector2& _scale);

	void SetIsDrawAll(bool _isDrawAll);
	void SetAlignment(Alignment _alignment);

	const Vector2& GetBasePosition() const;
	float GetDigitSpacing() const;
	int32_t GetNumDigits() const;

private:

	const std::string kFilePath_;

	std::vector<std::unique_ptr<Sprite>> numSprites_;

	/// 位置決定に使う変数
	Vector2 basePosition_;
	Vector2 baseSize_;
	float digitSpacing_ = 32.0f;

	size_t maxNumDigit_ = 0;
	int32_t currentNum_ = 0;
	int32_t digitNum_ = 0;
	int32_t previousNum_ = -1;

	Vector2 texSize_;
	Vector2 size_;

	bool isDrawAll_; // 全桁表示するかどうか
	Alignment alignment_ = Alignment::Center; /// 位置揃え
};

