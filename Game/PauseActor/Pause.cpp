#include "Pause.h"

/// engine
#include "input/Input.h"
#include "base/TextureManager.h"
#include "Scene/Manager/SceneManager.h"
#include "audio/Audio.h"

#include "Option/GameOption.h"

void Pause::Init() {
	isPause_ = false;


	std::vector<std::string> paths = {
		"./resources/Texture/Pause/PauseResume.png",
		"./resources/Texture/Pause/PauseReturnTitle.png",
		"./resources/Texture/Pause/OpenOption.png"
	};

	Vector2 offset = { 0.0f, 120.0f };
	Vector2 startPos = { 640.0f, 360.0f - offset.y };

	for (size_t i = 0; i < paths.size(); ++i) {
		Item newItem = std::make_unique<PauseMenuItem>(paths[i], i);
		newItem->SetPosition(startPos + offset * static_cast<float>(i));
		menuItems_.emplace_back(std::move(newItem));
	}

	uint32_t texHandle = TextureManager::GetInstance()->LoadTexture("./resources/Texture/default.png");
	background_.reset(Sprite::Create(
		texHandle, {},
		{ 1.0f, 1.0f, 1.0f, 0.5f }
	));

	background_->SetScale({ 1280.0f, 720.0f });

}

void Pause::Update() {
	Input* input = Input::GetInstance();
	GameOption* option = GameOption::GetInstance();

	if (!option->GetIsOpen()) {
		if (input->TrrigerKey(DIK_ESCAPE) ||
			input->IsTriggerPad(0, Gamepad::Start)) {
			/// SEの再生
			int seSoundId = Audio::GetInstance()->LoadWave("./resources/Sound/the_tmp.wav");
			Audio::GetInstance()->PlayWave(seSoundId, 0.2f);

			isPause_ = !isPause_;
		}
	}

	if (!isPause_) { return; }

	/// メニュー操作
	if (!option->GetIsOpen()) {
		if (input->TrrigerKey(DIK_UP) ||
			input->TrrigerKey(DIK_W) ||
			input->IsTriggerPad(0, Gamepad::DPadUp)) {
			/// SEの再生
			int seSoundId = Audio::GetInstance()->LoadWave("./resources/Sound/the_tmp.wav");
			Audio::GetInstance()->PlayWave(seSoundId, 0.2f);

			if (currentIndex_ == 0) {
				currentIndex_ = menuItems_.size() - 1;
			} else {
				--currentIndex_;
			}
		}

		if (input->TrrigerKey(DIK_DOWN) ||
			input->TrrigerKey(DIK_S) ||
			input->IsTriggerPad(0, Gamepad::DPadDown)) {
			/// SEの再生
			int seSoundId = Audio::GetInstance()->LoadWave("./resources/Sound/the_tmp.wav");
			Audio::GetInstance()->PlayWave(seSoundId, 0.2f);


			if (currentIndex_ == menuItems_.size() - 1) {
				currentIndex_ = 0;
			} else {
				++currentIndex_;
			}
		}

		/// 決定
		if (input->TrrigerKey(DIK_SPACE) ||
			input->IsTriggerPad(0, Gamepad::A)) {
			/// SEの再生
			int seSoundId = Audio::GetInstance()->LoadWave("./resources/Sound/the_tmp.wav");
			Audio::GetInstance()->PlayWave(seSoundId, 0.2f);

			switch (currentIndex_) {
			case Resume: // Resume
				isPause_ = false;
				break;
			case ReturnTitle: // Return Title
				/// タイトルに戻る処理
				isSceneChange_ = true;
				break;
			case OpenOption: // Open Option
				/// オプションを開く処理
				GameOption::GetInstance()->Open();
				break;
			}
		}
	}


	/// メニュー更新
	for (auto& item : menuItems_) {
		item->Update(currentIndex_);
	}

}

void Pause::Draw() {
	background_->Draw();
	for (auto& item : menuItems_) {
		item->Draw();
	}
}

bool Pause::IsPause() const {
	return isPause_;
}

bool Pause::IsSceneChange() const {
	return isSceneChange_;
}
