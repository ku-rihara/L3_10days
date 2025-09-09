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
		OPERATION_OPTION,
		SOUND_OPTION,
	};

public:
	/// ==================================
	/// public : methods
	/// ==================================
	
	static GameOption* GetInstance();

	void Load();
	void Save();

	void Init();
	void Update();
	void Draw();

	float GetMasterVolume() const;
	float GetBGMVolume() const;
	float GetSEVolume() const;

	void Open();
	void Close();

	bool GetIsOpen() const;
	bool GetIsDirtyThisFrame() const;
	bool GetPrevIsDirtyThisFrame() const;

private:
	/// ==================================
	/// private : objects
	/// ==================================

	bool isInitialized_ = false;
	bool isDirtyThisFrame_, prevIsDirtyThisFrame_;
	bool isSelectedItem_;
	bool isOpen_ = false;
	size_t currentIndex_ = 0;
	
	using Item = std::unique_ptr<IGameOptionItem>;
	std::vector<Item> menuItems_;


	std::unique_ptr<Sprite> background_;
	std::unique_ptr<Sprite> selectedFrame_;

};

