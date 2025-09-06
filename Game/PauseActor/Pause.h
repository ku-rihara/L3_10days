#pragma once

/// std
#include <memory>
#include <vector>

/// pause menu
#include "Item/PauseMenuItem.h"

/// ///////////////////////////////////////////////////////////
/// ポーズ画面の表示用クラス
/// ///////////////////////////////////////////////////////////
class Pause {
public:
	/// =========================================================
	/// public : methods
	/// =========================================================

	Pause() = default;
	~Pause() = default;
	
	void Init();
	void Update();
	void Draw();

	bool IsPause() const;

private:
	/// =========================================================
	/// private : objects
	/// =========================================================

	bool isPause_ = false;
	size_t currentIndex_ = 0;

	using Item = std::unique_ptr<PauseMenuItem>;
	std::vector<Item> menuItems_;

	std::unique_ptr<Sprite> background_ = nullptr;

};

