#include "Pause.h"

#include "input/Input.h"

void Pause::Init() {
	isPause_ = false;


	std::vector<std::string> paths = {
		"./resources/Texture/Pause/PauseResume.png",
		"./resources/Texture/Pause/PauseReturnTitle.png",
		"./resources/Texture/Pause/OpenOption.png"
	};

	Vector2 startPos = { 640.0f, 200.0f };
	Vector2 offset = { 0.0f, 100.0f };

	for (size_t i = 0; i < paths.size(); ++i) {
		Item newItem = std::make_unique<PauseMenuItem>(paths[i], i);
		newItem->SetPosition(startPos + offset * static_cast<float>(i));
		menuItems_.emplace_back(std::move(newItem));
	}

}

void Pause::Update() {
	Input* input = Input::GetInstance();
	if (input->TrrigerKey(DIK_ESCAPE)) {
		isPause_ = !isPause_;
	}

	if (!isPause_) { return; }

	/// メニュー操作
	if (input->TrrigerKey(DIK_UP)) {
		if (currentIndex_ == 0) {
			currentIndex_ = menuItems_.size() - 1;
		} else {
			--currentIndex_;
		}
	}

	if (input->TrrigerKey(DIK_DOWN)) {
		if (currentIndex_ == menuItems_.size() - 1) {
			currentIndex_ = 0;
		} else {
			++currentIndex_;
		}
	}

	
	/// メニュー更新
	for(auto& item : menuItems_) {
		item->Update(currentIndex_);
	}

}

void Pause::Draw() {
	for(auto& item : menuItems_) {
		item->Draw();
	}
}

bool Pause::IsPause() const {
	return isPause_;
}
