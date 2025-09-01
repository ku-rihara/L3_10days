#pragma once

/// base
#include "BaseScene.h"
#include "SkyBox/SkyBox.h"

/// game
#include "../Actor/Boundary/Boundary.h"


class GameScene : public BaseScene {
private:
	///========================================================
	/// Private variants
	///========================================================

	std::unique_ptr<SkyBox> skuBox_ = nullptr;

	/// game objects
	Boundary* boundary_ = nullptr;


public:
	///========================================================
	/// Constralt destract
	///========================================================
	GameScene();
	~GameScene() override;

	///========================================================
	/// private method
	///========================================================

	/// 初期化、更新、描画
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