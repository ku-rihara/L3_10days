#include "GameOverScene.h"

#include "Scene/Manager/SceneManager.h"

/// actors
#include "GameOverActor/GameOverSprite.h"
#include "Option/GameOption.h"

GameOverScene::GameOverScene() = default;
GameOverScene::~GameOverScene() = default;

void GameOverScene::Init() {
	BaseScene::Init();
	GameOption::GetInstance()->Init();

	/// インスタンスの生成
	gameOverSprite_ = std::make_unique<GameOverSprite>();

	/// 各種初期化
	gameOverSprite_->Init();

	bgmId_ = audio_->LoadWave("./resources/Sound/BGM/GameOverBGM.wav");
	audio_->PlayBGM(bgmId_, 0.1f);

	fade_ = std::make_unique<Fade>();
	fade_->Init();
}

void GameOverScene::Update() {

	fade_->Update();

	if (!fade_->IsFade()) {
		gameOverSprite_->Update();
		/// 選択したときの処理
		if (input_->TrrigerKey(DIK_RETURN) ||
			input_->TrrigerKey(DIK_SPACE) ||
			input_->IsTriggerPad(0, Gamepad::A)) {

			/// 効果音の再生
			int soundId = audio_->LoadWave("./resources/Sound/SE/DecideSE.wav");
			audio_->PlayWave(soundId, 0.1f);

			fade_->FadeOut(1.2f);
		}
	}


	if (fade_->IsFadeEnd()) {
		///  bgm stop
		audio_->StopBGM(bgmId_);

		switch (gameOverSprite_->GetSelectIndex()) {
		case GameOverSprite::RetryGame:
			SceneManager::GetInstance()->ChangeScene("GAMEPLAY");
			return;
		case GameOverSprite::ReturnTitle:
			SceneManager::GetInstance()->ChangeScene("TITLE");
			return;
		}
	}

}

void GameOverScene::ModelDraw() {}

void GameOverScene::SpriteDraw() {
	gameOverSprite_->Draw();
	fade_->Draw();
}

void GameOverScene::SkyBoxDraw() {}

void GameOverScene::DrawShadow() {}

void GameOverScene::Debug() {}

void GameOverScene::ViewProjectionUpdate() {}

void GameOverScene::ViewProssess() {}

