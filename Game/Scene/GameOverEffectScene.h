#pragma once

/// engine
#include "BaseScene.h"

class GameOverEffectScene : public BaseScene {


public:
	GameOverEffectScene();
	~GameOverEffectScene() override;
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

