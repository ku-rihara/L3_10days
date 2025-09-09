#include "OperationOptionItem.h"

#include "imgui.h"
#include "json.hpp"

/// std
#include <fstream>
#include <iomanip>

/// engine
#include "base/TextureManager.h"
#include "Frame/Frame.h"
#include "input/Input.h"
#include "audio/Audio.h"

void OperationOptionItem::Init() {

	uint32_t texHandle = TextureManager::GetInstance()->LoadTexture(
		"./resources/Texture/Option/Controller.png"
	);

	controllerSprite_.reset(Sprite::Create(
		texHandle,
		{ 860.0f, 360.0f }, { 1, 1, 1, 1.0f })
	);

	controllerSprite_->anchorPoint_ = { 0.5f, 0.5f };
	controllerSprite_->SetScale(Vector2(1, 1) / 2.25f);


	/// Frame
	texHandle = TextureManager::GetInstance()->LoadTexture(
		"./resources/Texture/Option/Check.png");
	frame_.reset(Sprite::Create(texHandle, { 640.0f, 360.0f }, { 1, 1, 1, 1.0f }));
	frame_->anchorPoint_ = { 0.5f, 0.5f };
	frame_->SetPosition({ 802.f, 584.f });
	frame_->SetScale(Vector2(1, 1) * 0.5f);

	/// Hint
	texHandle = TextureManager::GetInstance()->LoadTexture(
		"./resources/Texture/Option/CheckBox.png");
	hint_.reset(Sprite::Create(texHandle, { 640.0f, 360.0f }, { 1, 1, 1, 1.0f }));
	hint_->anchorPoint_ = { 0.5f, 0.5f };
	hint_->SetPosition({ 800.f, 591.f });
	hint_->SetScale(Vector2(1, 1) * 0.25f);

	/// time
	time_ = 0.0f;

	active_ = ReadJson();
}

void OperationOptionItem::Update(size_t _currentIndex, bool _isDirtyThisFrame) {
	time_ += Frame::DeltaTime();
	float scale = std::sin(time_ * 3.0f) * 0.5f + 0.5f;
	hint_->SetScale(Vector2(1, 1) * (0.25f + scale * 0.1f));

	/// currentIndexとthisIndexが違ったらreturn
	if (thisIndex_ != _currentIndex) {
		return;
	}

	if (_isDirtyThisFrame) {
		return;
	}

	Input* input = Input::GetInstance();
	if (input->TrrigerKey(DIK_SPACE) ||
		input->TrrigerKey(DIK_RETURN) ||
		input->IsTriggerPad(0, Gamepad::A)) {
		/// 決定音
		Audio* audio = Audio::GetInstance();
		int seSoundId = audio->LoadWave("./resources/Sound/SE/DecideSE.wav");
		audio->PlayWave(seSoundId, 0.1f);

		/// ファイルを開く
		/// GameOption.jsonを開く
		active_ = !active_;
		WriteJson(active_);
	}

}

void OperationOptionItem::Draw() {
	controllerSprite_->Draw();
	hint_->Draw();


	if (active_) {
		frame_->Draw();
	}
}

void OperationOptionItem::WriteJson(bool _active) {
	/// Jsonに書き込む
	nlohmann::json jsonData;
	jsonData["inversePitch"] = _active;
	std::ofstream file("./resources/Option/Operation.json");
	file << std::setw(4) << jsonData << std::endl;
	file.close();
}

bool OperationOptionItem::ReadJson() {
	/// Jsonを読む
	std::ifstream file("./resources/Option/Operation.json");
	if (!file.is_open()) {
		return false;
	}
	nlohmann::json jsonData;
	file >> jsonData;
	file.close();

	return jsonData.value("inversePitch", false);
}
