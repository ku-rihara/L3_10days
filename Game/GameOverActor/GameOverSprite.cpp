#include "GameOverSprite.h"

/// engine
#include "base/TextureManager.h"
#include "input/Input.h"

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

	{	/// selected frame
		uint32_t textureHandle = TextureManager::GetInstance()->LoadTexture(
			"./resources/Texture/GameResult/GameOverSelectUI.png");
		selectedFrame_.reset(Sprite::Create(textureHandle, Vector2(0, 0), { 1, 1, 1, 1 }));
		selectedFrame_->anchorPoint_ = { 0.5f, 0.5f };
		selectedFrame_->SetPosition({ 640.0f, 500.0f });
	}

	{	/// icons
		const size_t maxIcons = 3;
		const std::vector<std::string> iconPaths = {
			"./resources/Texture/GameResult/GameOverTextCheckPoint.png",
			"./resources/Texture/GameResult/GameOverTextRetryGame.png",
			"./resources/Texture/GameResult/GameOverTextReturnTitle.png"
		};

		float offset = 60.0f;
		float posY = 680.0f - 150.0f - offset;

		for (size_t i = 0; i < maxIcons; i++) {
			auto& icon = itemIcons_.emplace_back();
			uint32_t textureHandle = TextureManager::GetInstance()->LoadTexture(iconPaths[i]);
			icon.reset(Sprite::Create(textureHandle, Vector2(0, 0), { 1, 1, 1, 1 }));
			icon->anchorPoint_ = { 0.0f, 1.0f };
			icon->SetPosition({ 80.0f, posY + i * offset });
		}

	}

}

void GameOverSprite::Update() {

	/// 更新
	Input* input = Input::GetInstance();
	if (input->TrrigerKey(DIK_W) ||
		input->TrrigerKey(DIK_UP)) {
		if (selectIndex_ > 0) {
			--selectIndex_;
		}
	}

	if (input->TrrigerKey(DIK_S) ||
		input->TrrigerKey(DIK_DOWN)) {
		if (selectIndex_ < 2) {
			++selectIndex_;
		}
	}

	selectedFrame_->SetPosition({ 640.0f, 300.0f + selectIndex_ * 100.0f });


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
