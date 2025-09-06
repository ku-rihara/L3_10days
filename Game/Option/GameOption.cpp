#include "GameOption.h"

/// std
#include <fstream>

/// json
#include <json.hpp>

/// engine
#include "base/TextureManager.h"
#include "input/Input.h"

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
	Vector2 startPos = { 640.0f, 360.0f };
	//Vector2 offset = { 0.0f, 100.0f };


	background_.reset(Sprite::Create(
		TextureManager::GetInstance()->LoadTexture("./resources/Texture/Option/Frame.png"),
		startPos, { 0.302f, 0.302f, 0.302f, 0.5f }
	));
	//Vector2 texSize = { 256.0f, 256.0f };
	//background_->SetScale(texSize / Vector2{ 512.0f, 512.0f });
	background_->anchorPoint_ = { 0.1f, 0.5f };

	/// サウンドオプションの生成
	std::unique_ptr<SoundOption> soundOption = std::make_unique<SoundOption>();
	soundOption->BaseInit("./resources/Texture/Option/SoundOption.png", SOUND_OPTION);
	soundOption->Init();
	menuItems_.emplace_back(std::move(soundOption));

}

void GameOption::Update() {

	/// Close
	Input* input = Input::GetInstance();
	if (input->TrrigerKey(DIK_ESCAPE)) {
		if (GetIsOpen()) {
			Close();
		}
	}

	if (!isOpen_) {
		return;
	}

	for (size_t i = 0; i < menuItems_.size(); i++) {
		menuItems_[i]->Update(currentIndex_);
	}

}

void GameOption::Draw() {
	if (!isOpen_) {
		return;
	}

	background_->Draw();

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

	/// save
	const std::string path = "./resources/Option/GameOption.json";
	nlohmann::json json;
	json["masterVolume"] = GetMasterVolume();
	json["bgmVolume"] = GetBGMVolume();
	json["seVolume"] = GetSEVolume();
	std::ofstream ofs(path);
	ofs << json.dump(4);
}

bool GameOption::GetIsOpen() const {
	return isOpen_;
}
