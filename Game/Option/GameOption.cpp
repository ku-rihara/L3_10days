#include "GameOption.h"

/// std
#include <fstream>

/// json
#include <json.hpp>

/// engine
#include "base/TextureManager.h"
#include "input/Input.h"
#include "audio/Audio.h"

/// option
#include "Item/SoundOption.h"
#include "Item/OperationOptionItem.h"

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

void GameOption::Save() {
	/// save
	const std::string path = "./resources/Option/GameOption.json";
	nlohmann::json json;
	json["masterVolume"] = GetMasterVolume();
	json["bgmVolume"] = GetBGMVolume();
	json["seVolume"] = GetSEVolume();
	std::ofstream ofs(path);
	ofs << json.dump(4);
}


void GameOption::Init() {
	if (isInitialized_) {
		return;
	}

	isInitialized_ = true;

	isOpen_ = false;
	Vector2 startPos = { 640.0f, 360.0f };

	/// 背景
	uint32_t bgTexHandle = TextureManager::GetInstance()->LoadTexture("./resources/Texture/default.png");
	background_.reset(Sprite::Create(
		bgTexHandle, { 640.0f, 360.0f }, { 0.25098f, 0.25098f, 0.25098f, 1.0f }));
	background_->anchorPoint_ = { 0.5f, 0.5f };
	background_->SetScale(Vector2{ 1280.0f, 720.0f } *0.4f);

	/// SelectedFrameの生成
	selectedFrame_.reset(Sprite::Create(
		TextureManager::GetInstance()->LoadTexture("./resources/Texture/Option/SelectedFrame.png"),
		{ 0.0f, 0.0f }, { 1, 1, 1, 1.0f }));
	selectedFrame_->anchorPoint_ = { 0.5f, 0.5f };
	selectedFrame_->SetScale({ 1.0f, 1.0f });


	/// 操作オプションの生成
	std::unique_ptr<OperationOptionItem> operationOption = std::make_unique<OperationOptionItem>();
	operationOption->BaseInit("./resources/Texture/Option/OperationOption.png", OPERATION_OPTION);
	operationOption->Init();
	menuItems_.emplace_back(std::move(operationOption));

	/// サウンドオプションの生成
	std::unique_ptr<SoundOption> soundOption = std::make_unique<SoundOption>();
	soundOption->BaseInit("./resources/Texture/Option/SoundOption.png", SOUND_OPTION);
	soundOption->Init();
	menuItems_.emplace_back(std::move(soundOption));

	isDirtyThisFrame_ = false;

	Load();

	/// 一回更新
	for (size_t i = 0; i < menuItems_.size(); i++) {
		menuItems_[i]->Update(currentIndex_);
	}
}

void GameOption::Update() {
	Input* input = Input::GetInstance();

	/// 同一フレーム内でのOpen, Closeを防止 and Itemを選択していないなら
	if (!isDirtyThisFrame_ && !isSelectedItem_) {
		/// Closeする処理
		if (input->TrrigerKey(DIK_ESCAPE) ||
			input->IsTriggerPad(0, Gamepad::Start)) {
			if (GetIsOpen()) {
				Close();
			}
		}
	}

	/// 開いていないなら更新しない
	if (!isOpen_) {
		return;
	}

	/// Itemを選択していないなら
	if (!isSelectedItem_) {

		if (input->TrrigerKey(DIK_UP) ||
			input->TrrigerKey(DIK_W) ||
			input->IsTriggerPad(0, Gamepad::DPadUp)) {

			if (currentIndex_ > 0) {
				currentIndex_--;
			}
		}

		if (input->TrrigerKey(DIK_DOWN) ||
			input->TrrigerKey(DIK_S) ||
			input->IsTriggerPad(0, Gamepad::DPadDown)) {

			if (currentIndex_ < menuItems_.size() - 1) {
				currentIndex_++;
			}
		}


		/// 選択しているItemの位置にFrameを移動
		Vector2 itemPos = menuItems_[currentIndex_]->GetStartPos() +
			menuItems_[currentIndex_]->GetOffsetPos() * static_cast<float>(currentIndex_);
		selectedFrame_->SetPosition(itemPos);

		/// 決定
		if (!isDirtyThisFrame_) {
			if (input->TrrigerKey(DIK_SPACE) ||
				input->IsTriggerPad(0, Gamepad::A)) {
				isSelectedItem_ = true;
				/// SEの再生
				int seSoundId = Audio::GetInstance()->LoadWave("./resources/Sound/SE/DecideSE.wav");
				Audio::GetInstance()->PlayWave(seSoundId, 0.1f);
			}
		}

	}


	if (!isDirtyThisFrame_) {
		if (input->TrrigerKey(DIK_ESCAPE) ||
			input->IsTriggerPad(0, Gamepad::B)) {
			isSelectedItem_ = false;
		}
	}

	if (isSelectedItem_) {
		for (size_t i = 0; i < menuItems_.size(); i++) {
			menuItems_[i]->Update(currentIndex_);
		}
	}

	isDirtyThisFrame_ = false;
}

void GameOption::Draw() {
	if (!isOpen_) {
		return;
	}

	background_->Draw();

	if (!isSelectedItem_) {
		selectedFrame_->Draw();
	}

	for (auto& item : menuItems_) {
		item->BaseDraw();
	}

	menuItems_[currentIndex_]->Draw();
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
	isDirtyThisFrame_ = true;

	/// SEの再生
	Audio* audio_ = Audio::GetInstance();
	int soundId = audio_->LoadWave("./resources/Sound/the_tmp.wav");
	audio_->PlayWave(soundId, 0.2f);
}

void GameOption::Close() {
	isOpen_ = false;
	isDirtyThisFrame_ = true;

	/// SEの再生
	Audio* audio_ = Audio::GetInstance();
	int soundId = audio_->LoadWave("./resources/Sound/the_tmp.wav");
	audio_->PlayWave(soundId, 0.2f);

	Save();
}

bool GameOption::GetIsOpen() const {
	return isOpen_;
}
