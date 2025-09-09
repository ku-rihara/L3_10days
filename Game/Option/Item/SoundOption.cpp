#include "SoundOption.h"

/// std
#include <algorithm>

/// engine
#include "base/TextureManager.h"
#include "input/Input.h"
#include "audio/Audio.h"
#include "Frame/Frame.h"

void SoundOption::Init() {

	std::vector<std::string> paths = {
		"./resources/Texture/Option/MasterVolumeOption.png",
		"./resources/Texture/Option/BGMVolumeOption.png",
		"./resources/Texture/Option/SEVolumeOption.png"
	};

	std::string backgroundPath = "./resources/Texture/Option/VolumeBackground.png";
	std::string sliderBGPath = "./resources/Texture/Option/VolumeSliderBackground.png";
	std::string volumeBarPath = "./resources/Texture/Option/VolumeBar.png";
	std::string sliderPath = "./resources/Texture/Option/VolumeSlider.png";

	Vector2 startPos = { 640.0f + 240.0f, 360.0f - 100.0f };
	Vector2 offset = { 0.0f, 100.0f };
	Vector2 scale = { 0.5f, 0.5f };
	Vector2 size = Vector2{ 512, 64 } *scale;

	soundItems_.resize(MAX);
	for (size_t i = 0; i < soundItems_.size(); ++i) {
		Vector2 pos = startPos + offset * static_cast<float>(i);
		soundItems_[i].pos = pos;
		soundItems_[i].size = size;
		soundItems_[i].textOffset = { -192.0f, 0.0f };

		/// text
		if (!soundItems_[i].textSprite_) {
			uint32_t textTexHandle = TextureManager::GetInstance()->LoadTexture(paths[i]);
			soundItems_[i].textSprite_.reset(Sprite::Create(
				textTexHandle,
				pos + soundItems_[i].textOffset,
				{ 1.0f, 1.0f, 1.0f, 1.0f }
			));
			soundItems_[i].textSprite_->anchorPoint_ = { 0.5f, 0.5f };
			soundItems_[i].textSprite_->SetScale(scale / 2.0f);
		}


		/// background
		if (!soundItems_[i].background) {
			uint32_t bgTexHandle = TextureManager::GetInstance()->LoadTexture(backgroundPath);
			soundItems_[i].background.reset(Sprite::Create(
				bgTexHandle, pos, { 1.0f, 1.0f, 1.0f, 1.0f }
			));
			soundItems_[i].background->anchorPoint_ = { 0.5f, 0.5f };
			soundItems_[i].background->SetScale(scale);
		}

		/// sliderBG
		if (!soundItems_[i].sliderBG) {
			uint32_t bgTexHandle = TextureManager::GetInstance()->LoadTexture(sliderBGPath);
			soundItems_[i].sliderBG.reset(Sprite::Create(
				bgTexHandle, pos, { 1.0f, 1.0f, 1.0f, 1.0f }
			));
			soundItems_[i].sliderBG->anchorPoint_ = { 0.5f, 0.5f };
			soundItems_[i].sliderBG->SetScale(scale);
		}

		if (!soundItems_[i].volumeBar) {
			uint32_t volumeBarTexHandle = TextureManager::GetInstance()->LoadTexture(volumeBarPath);
			soundItems_[i].volumeBar.reset(Sprite::Create(
				volumeBarTexHandle,
				pos - Vector2(size.x / 2.0f, 0.0f),
				{ 1.0f, 1.0f, 1.0f, 1.0f }
			));
			soundItems_[i].volumeBar->anchorPoint_ = { 0.0f, 0.5f };
			soundItems_[i].volumeBar->SetScale(scale);
		}

		/// slider
		if (!soundItems_[i].slider) {
			uint32_t sliderTexHandle = TextureManager::GetInstance()->LoadTexture(sliderPath);
			soundItems_[i].slider.reset(Sprite::Create(
				sliderTexHandle, pos, { 1.0f, 1.0f, 1.0f, 1.0f }
			));
			soundItems_[i].slider->anchorPoint_ = { 0.5f, 0.5f };
			soundItems_[i].slider->SetScale(scale);
		}

		/// numDraw
		if (!soundItems_[i].volumeNum) {
			soundItems_[i].volumeNum = std::make_unique<NumDraw>();
			soundItems_[i].volumeNum->Init(3);
			soundItems_[i].volumeNum->SetBasePosition(pos + Vector2{ 192.0f, 0.0f });
			soundItems_[i].volumeNum->SetDigitSpacing(16.0f);
			soundItems_[i].volumeNum->SetNumber(static_cast<int32_t>(soundItems_[i].volume * 100.0f));
			soundItems_[i].volumeNum->SetScale({ 2.0f, 2.0f });
			soundItems_[i].volumeNum->SetColor({ 1.0f, 1.0f, 1.0f, 1.0f });
		}
	}


	/// selected frame
	selectedFrame_.reset(Sprite::Create(
		TextureManager::GetInstance()->LoadTexture("./resources/Texture/Option/SelectedFrame.png"),
		{ 0.0f, 0.0f }, { 1.0f, 1.0f, 1.0f, 1.0f }
	));

	selectedFrame_->SetScale({ 150.0f, 150.0f });
	selectedFrame_->anchorPoint_ = { 0.5f, 0.5f };
	selectedFrame_->SetScale(scale);

	isSelected_ = false;
}

void SoundOption::Update(size_t _currentIndex) {

	if (thisIndex_ != _currentIndex) {
		return;
	}

	/// 音量調整
	/// Mouseの座標を取得
	Input* input = Input::GetInstance();
	Vector2 mousePos = input->GetMousePos();


	/// volumeに合わせてsliderの位置を変更
	for (auto& item : soundItems_) {
		float volume = item.volume;
		Vector2 framePos = item.pos;
		framePos.x += (volume - 0.5f) * item.size.x;
		item.slider->SetPosition(framePos);

		/// volumeBarの大きさを変更
		float scaleY = item.volumeBar->transform_.scale.y;
		item.volumeBar->SetScale({ 0.5f * volume, scaleY });

		/// volumeNumの更新
		item.volumeNum->SetNumber(static_cast<int32_t>(item.volume * 100.0f));
		item.volumeNum->Update();
	}

	/// 入力で選択しているItemを変更
	if (!isSelected_) {
		if (input->TrrigerKey(DIK_UP) ||
			input->TrrigerKey(DIK_W) ||
			input->IsTriggerPad(0, Gamepad::DPadUp)) {
			/// SEの再生
			int seSoundId = Audio::GetInstance()->LoadWave("./resources/Sound/SE/SelectSE.wav");
			Audio::GetInstance()->PlayWave(seSoundId, 0.2f);
			if (selectedIndex_ > 0) {
				selectedIndex_--;
			}
		}

		if (input->TrrigerKey(DIK_DOWN) ||
			input->TrrigerKey(DIK_S) ||
			input->IsTriggerPad(0, Gamepad::DPadDown)) {
			/// SEの再生
			int seSoundId = Audio::GetInstance()->LoadWave("./resources/Sound/SE/SelectSE.wav");
			Audio::GetInstance()->PlayWave(seSoundId, 0.2f);
			selectedIndex_ = (selectedIndex_ + 1) % soundItems_.size();
		}
	}


	if (input->TrrigerKey(DIK_SPACE) ||
		input->IsTriggerPad(0, Gamepad::A)) {
		/// SEの再生
		int seSoundId = Audio::GetInstance()->LoadWave("./resources/Sound/SE/DecideSE.wav");
		Audio::GetInstance()->PlayWave(seSoundId, 0.2f);
		selectedIndex_ = (selectedIndex_) % soundItems_.size();
		isSelected_ = !isSelected_;
	}


	/// 左右で音量を変更
	if (isSelected_) {

		/// 減らす
		static bool isDownInput = false;
		static float downInterval = 0.3f;
		if (input->TrrigerKey(DIK_A) ||
			input->TrrigerKey(DIK_LEFT) ||
			input->IsTriggerPad(0, Gamepad::DPadLeft)) {
			float volume = soundItems_[selectedIndex_].volume;
			volume = std::clamp(volume - 0.01f, 0.0f, 1.0f);
			soundItems_[selectedIndex_].volume = volume;
			isDownInput = true;
		}

		if (isDownInput && input->TrrigerKey(DIK_A) || input->TrrigerKey(DIK_LEFT) ||
			input->IsPressPad(0, Gamepad::DPadLeft)) {
			downInterval -= Frame::DeltaTime();
			if(downInterval < 0.0f) {
				float volume = soundItems_[selectedIndex_].volume;
				volume = std::clamp(volume - 0.005f, 0.0f, 1.0f);
				soundItems_[selectedIndex_].volume = volume;
			}
		} else {
			isDownInput = false;
			downInterval = 0.3f;
		}


		/// 増やす
		static bool isUpInput = false;
		static float upInterval = 0.3f;
		if (input->TrrigerKey(DIK_S) ||
			input->TrrigerKey(DIK_RIGHT) ||
			input->IsTriggerPad(0, Gamepad::DPadRight)) {
			float volume = soundItems_[selectedIndex_].volume;
			volume = std::clamp(volume + 0.01f, 0.0f, 1.0f);
			soundItems_[selectedIndex_].volume = volume;
			isUpInput = true;
		}

		/// 長押しで増やす
		if (isUpInput && input->TrrigerKey(DIK_S) || input->TrrigerKey(DIK_RIGHT) ||
			input->IsPressPad(0, Gamepad::DPadRight)) {
			upInterval -= Frame::DeltaTime();
			if (upInterval < 0.0f) {
				float volume = soundItems_[selectedIndex_].volume;
				volume = std::clamp(volume + 0.005f, 0.0f, 1.0f);
				soundItems_[selectedIndex_].volume = volume;
			}
		} else {
			isUpInput = false;
			upInterval = 0.3f;
		}
	}


	/// 

	///// Mouseの左クリックが押されているか
	//if (input->IsPressMouse(0)) {
	//	/// 音量を設定
	//	Vector2 min = soundItems_[selectedIndex_].pos - soundItems_[selectedIndex_].size * 0.5f;
	//	Vector2 max = soundItems_[selectedIndex_].pos + soundItems_[selectedIndex_].size * 0.5f;
	//	float volume = (mousePos.x - min.x) / (max.x - min.x);
	//	volume = std::clamp(volume, 0.0f, 1.0f);
	//	soundItems_[selectedIndex_].volume = volume;
	//} else {

	//	/// 選択しているSoundItemの範囲内にMouseがあるか
	//	for (size_t i = 0; i < soundItems_.size(); i++) {
	//		auto& item = soundItems_[i];
	//		Vector2 min = item.pos - item.size * Vector2(0.5f, 0);
	//		Vector2 max = item.pos + item.size * Vector2(0.5f, 0);
	//		if (mousePos.x >= min.x && mousePos.x <= max.x &&
	//			mousePos.y >= min.y && mousePos.y <= max.y) {
	//			selectedIndex_ = i;
	//			break;
	//		}
	//	}
	//}


	/// indexの位置にFrameを表示
	Vector2 framePos = soundItems_[selectedIndex_].pos;
	float volume = soundItems_[selectedIndex_].volume;
	framePos.x += (volume - 0.5f) * soundItems_[selectedIndex_].size.x;
	selectedFrame_->SetPosition(framePos);

}


void SoundOption::Draw() {
	for (auto& item : soundItems_) {
		/// 背景
		item.background->Draw();

		/// text
		item.textSprite_->Draw();
		item.volumeNum->Draw();

		/// slider
		item.sliderBG->Draw();
		item.volumeBar->Draw();
		item.slider->Draw();
	}

	selectedFrame_->Draw();
}

void SoundOption::SetVolume(ItemName _name, float _volume) {
	soundItems_[_name].volume = _volume;
}

float SoundOption::GetVolume(ItemName _name) const {
	return soundItems_[_name].volume;
}
