#pragma once

/// std
#include <memory>
#include <vector>

/// base
#include "BaseScene.h"
#include "BaseObject/BaseObject.h"

class GameClearEffectScene : public BaseScene {

	std::vector<std::unique_ptr<BaseObject>> sceneObj_;

public:
	GameClearEffectScene();
	~GameClearEffectScene() override;
	void Init() override;
	void Update() override;
	void ModelDraw() override;
	void SpriteDraw() override;
	void SkyBoxDraw() override;
	void DrawShadow() override;
	void ViewProssess() override;
};

