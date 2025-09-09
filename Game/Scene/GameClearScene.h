#pragma once

/// std
#include <memory>

/// base
#include "BaseScene.h"

class GameClearScene : public BaseScene {
private:
	/// ========================================================
	/// private objects
	/// ========================================================

	std::unique_ptr<class GameClearSprite> gameClearSprite_; ///< ゲームクリアスプライト
	int bgmId_;

public:
	/// ========================================================
	/// public : methods
	/// ========================================================

	GameClearScene();
	~GameClearScene() override;

	void Init() override;
	void Update() override;
	void ModelDraw() override;
	void SpriteDraw() override;
	void SkyBoxDraw() override;
	void DrawShadow() override;
	void Debug() override; /// debug
	void ViewProjectionUpdate() override;
	void ViewProssess() override;
};

