#pragma once

/// std
#include <memory>
#include <vector>

/// engine
#include "2d/Sprite.h"

/// ////////////////////////////////////////////////////////////////////
/// @brief  ゲームオーバースプライト
/// ////////////////////////////////////////////////////////////////////
class GameOverSprite {
public:
	using USprite = std::unique_ptr<Sprite>;

	enum SelectedItem {
		RetryGame,
		ReturnTitle,
	};;

public:
	/// ==============================================================
	/// public : methods
	/// ==============================================================

	GameOverSprite();
	~GameOverSprite();

	void Init();
	void Update();
	void Draw();

	size_t GetSelectIndex() const;

private:
	/// ==============================================================
	/// private : methods
	/// ==============================================================

	USprite background_;
	USprite gameOverText_;
	USprite itemFrame_;
	USprite selectedFrame_;

	std::vector<USprite> itemIcons_;
	size_t selectIndex_ = 0;
	const size_t kMaxIcons_ = 2;

	Vector2 startPos_;
	Vector2 offset_;

};

