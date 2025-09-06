#pragma once

/// base
#include "GameOptionItem.h"

class SoundOption : public IGameOptionItem {
public:
	
	struct SoundItem {
		float volume = 0.5f;

		Vector2 pos;
		Vector2 size;

		std::string texFilePath;
		size_t thisIndex = 0;

		Vector2 textOffset = { 10.0f, -10.0f };

		std::unique_ptr<Sprite> textSprite_ = nullptr;
		std::unique_ptr<Sprite> background = nullptr;
		std::unique_ptr<Sprite> slider = nullptr;
	};

	enum ItemName {
		MASTER_VOLUME = 0,
		BGM_VOLUME,
		SE_VOLUME,
		MAX
	};

public:

	SoundOption() = default;
	~SoundOption() override = default;
	
	void Init() override;
	void Update(size_t _currentIndex) override;
	void Draw() override;

	void SetVolume(ItemName _name, float _volume);
	float GetVolume(ItemName _name) const;

private:
	/// ==================================
	/// private : objects
	/// ==================================

	std::vector<SoundItem> soundItems_;
	size_t selectedIndex_ = 0;

	std::unique_ptr<Sprite> selectedFrame_ = nullptr;

};

