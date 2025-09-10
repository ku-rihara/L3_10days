#pragma once

/// std
#include <memory>

/// base
#include "BaseScene.h"


class GameClearEffectScene : public BaseScene {


public:
	GameClearEffectScene();
	~GameClearEffectScene() override;
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

