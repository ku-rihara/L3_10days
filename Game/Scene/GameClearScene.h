#pragma once

/// base
#include "BaseScene.h"

class GameClearScene : public BaseScene {
private:
	/// ========================================================
	/// private objects
	/// ========================================================

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

