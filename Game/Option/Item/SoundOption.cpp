#include "SoundOption.h"

/// engine
#include "base/TextureManager.h"

void SoundOption::Init() {

	std::vector<std::string> paths = {
		"./resources/Texture/Option/MasterVolumeOption.png",
		"./resources/Texture/Option/BGMVolumeOption.png",
		"./resources/Texture/Option/SEVolumeOption.png"
	};

	std::string backgroundPath = "./resources/Texture/Option/VolumeBackground.png";
	std::string sliderPath = "./resources/Texture/Option/VolumeSlider.png";

	Vector2 startPos = { 640.0f, 200.0f };
	Vector2 offset = { 0.0f, 100.0f };

	soundItems_.resize(MAX);
	for (size_t i = 0; i < soundItems_.size(); ++i) {
		Vector2 pos = startPos + offset * static_cast<float>(i);
		soundItems_[i].pos = pos;
		soundItems_[i].size = { 400.0f, 50.0f };

		/// text
		if (!soundItems_[i].textSprite_) {
			uint32_t textTexHandle = TextureManager::GetInstance()->LoadTexture(paths[i]);
			soundItems_[i].textSprite_ = std::make_unique<Sprite>();
			soundItems_[i].textSprite_.reset(Sprite::Create(
				textTexHandle,
				pos + soundItems_[i].textOffset,
				{ 1.0f, 1.0f, 1.0f, 1.0f }
			));
			soundItems_[i].textSprite_->anchorPoint_ = { 0.0f, 0.5f };
		}


		/// background
		if (!soundItems_[i].background) {
			uint32_t bgTexHandle = TextureManager::GetInstance()->LoadTexture(backgroundPath);
			soundItems_[i].background = std::make_unique<Sprite>();
			soundItems_[i].background.reset(Sprite::Create(
				bgTexHandle,
				pos, { 1.0f, 1.0f, 1.0f, 1.0f }
			));
			soundItems_[i].background->SetScale(soundItems_[i].size);
			soundItems_[i].background->anchorPoint_ = { 0.5f, 0.5f };
		}

		/// slider
		if (!soundItems_[i].slider) {
			uint32_t sliderTexHandle = TextureManager::GetInstance()->LoadTexture(sliderPath);
			soundItems_[i].slider = std::make_unique<Sprite>();
			soundItems_[i].slider.reset(Sprite::Create(
				sliderTexHandle,
				pos, { 1.0f, 1.0f, 1.0f, 1.0f }
			));
			soundItems_[i].slider->SetScale({ 20.0f, 50.0f });
			soundItems_[i].slider->anchorPoint_ = { 0.5f, 0.5f };
		}
	}

}

void SoundOption::Update(size_t _currentIndex) {

	if(thisIndex_ != _currentIndex){
		return;
	}

	/// 音量調整
	/// Mouseの座標を取得
	
}


void SoundOption::Draw() {

	for (auto& item : soundItems_) {
		item.background->Draw();
		item.slider->Draw();
		item.textSprite_->Draw();
	}

}

void SoundOption::SetVolume(ItemName _name, float _volume) {
	soundItems_[_name].volume = _volume;
}

float SoundOption::GetVolume(ItemName _name) const {
	return soundItems_[_name].volume;
}
