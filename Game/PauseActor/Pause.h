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

private:
	/// =========================================================
	/// private : objects
	/// =========================================================

	bool isPause_ = false;

	using Item = std::unique_ptr<PauseMenuItem>;
	std::vector<Item> items_;

};

