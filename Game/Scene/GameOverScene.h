#pragma once

/// std
#include <memory>

/// base
#include "BaseScene.h"


/// //////////////////////////////////////////////////////////
/// ゲームオーバーシーン
/// //////////////////////////////////////////////////////////
class GameOverScene : public BaseScene {
private:
	/// ========================================================
	/// private objects
	/// ========================================================

	std::unique_ptr<class GameOverSprite> gameOverSprite_; ///< ゲームオーバースプライト

public:
	GameOverScene();
	~GameOverScene() override;
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

