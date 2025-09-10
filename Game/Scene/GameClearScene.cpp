#include "GameClearScene.h"

/// engine
#include "Scene/Manager/SceneManager.h"

/// actors
#include "GameClearActor/GameClearSprite.h"
#include "Option/GameOption.h"

GameClearScene::GameClearScene() = default;
GameClearScene::~GameClearScene() = default;

void GameClearScene::Init() {
	BaseScene::Init();
	GameOption::GetInstance()->Init();

	/// インスタンスの生成
	gameClearSprite_ = std::make_unique<GameClearSprite>();

	/// 各種初期化
	gameClearSprite_->Init();

	fade_ = std::make_unique<Fade>();
	fade_->Init();

	bgmId_ = audio_->LoadWave("./resources/Sound/BGM/ResultBGM.wav");
	audio_->PlayBGM(bgmId_, 0.1f);
}

void GameClearScene::Update() {
	gameClearSprite_->Update();

	fade_->Update();

	if (!fade_->IsFade()) {
		/// タイトルに戻る処理
		if (input_->TrrigerKey(DIK_RETURN) ||
			input_->TrrigerKey(DIK_SPACE) ||
			input_->IsTriggerPad(0, Gamepad::A)) {

			fade_->FadeOut(1.2f);

			/// 効果音の再生
			int soundId = audio_->LoadWave("./resources/Sound/SE/DecideSE.wav");
			audio_->PlayWave(soundId, 0.1f);
		}
	}


	if (fade_->IsFadeEnd()) {
		/// bgm stop
		audio_->StopBGM(bgmId_);
		SceneManager::GetInstance()->ChangeScene("TITLE");
		return;
	}


}

void GameClearScene::ModelDraw() {}

void GameClearScene::SpriteDraw() {
	gameClearSprite_->Draw();
	fade_->Draw();
}

void GameClearScene::SkyBoxDraw() {}

void GameClearScene::DrawShadow() {}

void GameClearScene::Debug() {}

void GameClearScene::ViewProjectionUpdate() {}

void GameClearScene::ViewProssess() {}

