#pragma once

/// std
#include <memory>

/// game
#include "Item/GameOptionItem.h"

/// ///////////////////////////////////////////////////////
/// ゲームオプション
/// ///////////////////////////////////////////////////////
class GameOption {
	~GameOption() = default;
	GameOption() = default;

	enum {
		SOUND_OPTION,
	};

public:
	/// ==================================
	/// public : methods
	/// ==================================
	
	static GameOption* GetInstance();

	void Load();

	void Init();
	void Update();
	void Draw();

	float GetMasterVolume() const;
	float GetBGMVolume() const;
	float GetSEVolume() const;

	void Open();
	void Close();

	bool GetIsOpen() const;

private:
	/// ==================================
	/// private : objects
	/// ==================================

	bool isInitialized_ = false;

	using Item = std::unique_ptr<IGameOptionItem>;
	std::vector<Item> menuItems_;

	size_t currentIndex_ = 0;
	bool isOpen_ = false;

};

