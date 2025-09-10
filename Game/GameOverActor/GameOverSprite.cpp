#include "GameOverSprite.h"

/// engine
#include "base/TextureManager.h"
#include "input/Input.h"
#include "audio/Audio.h"

GameOverSprite::GameOverSprite() = default;
GameOverSprite::~GameOverSprite() = default;

void GameOverSprite::Init() {
	{	/// 背景
		uint32_t textureHandle = TextureManager::GetInstance()->LoadTexture(
			"./resources/Texture/default.png");
		background_.reset(Sprite::Create(textureHandle, Vector2(0, 0), { 1, 1, 1, 1 }));
		background_->SetScale({ 1280.0f, 720.0f });
	}

	{	/// text
		uint32_t textureHandle = TextureManager::GetInstance()->LoadTexture(
			"./resources/Texture/GameResult/GameOverText.png");
		gameOverText_.reset(Sprite::Create(textureHandle, Vector2(0, 0), { 1, 1, 1, 1 }));
		gameOverText_->anchorPoint_ = { 0.5f, 0.5f };
		gameOverText_->SetPosition({ 640.0f, 200.0f });
	}

	{	/// item frame
		uint32_t textureHandle = TextureManager::GetInstance()->LoadTexture(
			"./resources/Texture/GameResult/GameOverBgUI.png");
		itemFrame_.reset(Sprite::Create(textureHandle, Vector2(0, 0), { 1, 1, 1, 1 }));
		itemFrame_->anchorPoint_ = { 0.0f, 1.0f };
		itemFrame_->SetPosition({ 40.0f, 680.0f });
	}

	offset_ = { 0.0f, 60.0f * 1.5f };
	startPos_ = { 65.0f, 515.0f };

	{	/// selected frame
		uint32_t textureHandle = TextureManager::GetInstance()->LoadTexture(
			"./resources/Texture/GameResult/GameOverSelectUI.png");
		selectedFrame_.reset(Sprite::Create(textureHandle, Vector2(0, 0), { 1, 1, 1, 1 }));
		selectedFrame_->anchorPoint_ = { 0.0f, 1.0f };
		selectedFrame_->SetPosition(startPos_);
	}

	{	/// icons
		const std::vector<std::string> iconPaths = {
			"./resources/Texture/GameResult/GameOverTextRetryGame.png",
			"./resources/Texture/GameResult/GameOverTextReturnTitle.png"
		};

		for (size_t i = 0; i < kMaxIcons_; i++) {
			auto& icon = itemIcons_.emplace_back();
			uint32_t textureHandle = TextureManager::GetInstance()->LoadTexture(iconPaths[i]);
			icon.reset(Sprite::Create(textureHandle, Vector2(0, 0), { 1, 1, 1, 1 }));
			icon->anchorPoint_ = { 0.0f, 1.0f };
			icon->SetPosition(startPos_ + offset_ * static_cast<float>(i));
		}
	}

}

void GameOverSprite::Update() {

	/// 更新
	Input* input = Input::GetInstance();
	Audio* audio = Audio::GetInstance();

	if (input->TrrigerKey(DIK_W) ||
		input->TrrigerKey(DIK_UP) ||
		input->IsTriggerPad(0, Gamepad::DPadUp)) {

		int soundId = audio->LoadWave("./resources/Sound/SE/SelectSE.wav");
		audio->PlayWave(soundId, 0.1f);

		if (selectIndex_ > 0) {
			--selectIndex_;
		}
	}

	if (input->TrrigerKey(DIK_S) ||
		input->TrrigerKey(DIK_DOWN) ||
		input->IsTriggerPad(0, Gamepad::DPadDown)) {

		int soundId = audio->LoadWave("./resources/Sound/SE/SelectSE.wav");
		audio->PlayWave(soundId, 0.1f);

		if (selectIndex_ < kMaxIcons_ - 1) {
			++selectIndex_;
		}
	}

	selectedFrame_->SetPosition(startPos_ + offset_ * static_cast<float>(selectIndex_));

}


void GameOverSprite::Draw() {

	/// 描画
	//background_->Draw();
	gameOverText_->Draw();
	itemFrame_->Draw();
	selectedFrame_->Draw();

	for (auto& icon : itemIcons_) {
		icon->Draw();
	}

}

size_t GameOverSprite::GetSelectIndex() const {
	return selectIndex_;
}
