#include "GameOption.h"

/// std
#include <fstream>

/// json
#include <json.hpp>

/// option
#include "Item/SoundOption.h"

GameOption* GameOption::GetInstance() {
	static GameOption instance;
	return &instance;
}

void GameOption::Load() {
	/// path
	const std::string path = "./resources/Option/GameOption.json";

	/// ファイル読み
	std::ifstream ifs(path);
	if (!ifs) {
		return;
	}

	/// json解析
	nlohmann::json json;
	ifs >> json;

	/// 各種設定
	float masterVolume = json.value("masterVolume", 0.5f);
	float bgmVolume = json.value("bgmVolume", 0.5f);
	float seVolume = json.value("seVolume", 0.5f);

	auto soundOption = static_cast<SoundOption*>(menuItems_[SOUND_OPTION].get());
	soundOption->SetVolume(SoundOption::MASTER_VOLUME, masterVolume);
	soundOption->SetVolume(SoundOption::BGM_VOLUME, bgmVolume);
	soundOption->SetVolume(SoundOption::SE_VOLUME, seVolume);
}


void GameOption::Init() {
	if (isInitialized_) {
		return;
	}

	isOpen_ = false;
	Vector2 startPos = { 640.0f, 200.0f };
	Vector2 offset = { 0.0f, 100.0f };

	/// サウンドオプションの生成
	std::unique_ptr<SoundOption> soundOption = std::make_unique<SoundOption>();
	soundOption->BaseInit("./resources/Texture/Option/SoundOption.png", SOUND_OPTION);
	soundOption->Init();
	menuItems_.emplace_back(std::move(soundOption));

}

void GameOption::Update() {

	if (!isOpen_) { return; }
	for (size_t i = 0; i < menuItems_.size(); i++) {
		menuItems_[i]->Update(i);
	}

}

void GameOption::Draw() {
	if (!isOpen_) {
		return;
	}

	for (auto& item : menuItems_) {
		item->Draw();
	}
}

float GameOption::GetMasterVolume() const {
	auto soundOption = static_cast<SoundOption*>(menuItems_[SOUND_OPTION].get());
	return soundOption->GetVolume(SoundOption::MASTER_VOLUME);
}

float GameOption::GetBGMVolume() const {
	auto soundOption = static_cast<SoundOption*>(menuItems_[SOUND_OPTION].get());
	return soundOption->GetVolume(SoundOption::BGM_VOLUME);
}

float GameOption::GetSEVolume() const {
	auto soundOption = static_cast<SoundOption*>(menuItems_[SOUND_OPTION].get());
	return soundOption->GetVolume(SoundOption::SE_VOLUME);
}

void GameOption::Open() {
	isOpen_ = true;
}

void GameOption::Close() {
	isOpen_ = false;
}

bool GameOption::GetIsOpen() const {
	return isOpen_;
}
