#pragma once

/// base
#include "Actor/GameCamera/GameCamera.h"
#include "Actor/Player/Player.h"
#include "BaseScene.h"
#include "SkyBox/SkyBox.h"
#include "Actor/Station/Enemy/EnemyStation.h"
#include"DebugGround/TestGround.h"

/// game
#include "../Actor/Boundary/Boundary.h"


class GameScene : public BaseScene {
private:
	///========================================================
	/// Private variants
	///========================================================

	std::unique_ptr<SkyBox> skuBox_ = nullptr;
	std::unique_ptr<Player> player_ = nullptr;
	std::unique_ptr<GameCamera> gameCamera_ = nullptr;
	std::unique_ptr<EnemyStation> enemyStation_ = nullptr;

	/// game objects
	Boundary* boundary_ = nullptr;

    std::unique_ptr<SkyBox> skuBox_         = nullptr;
    std::unique_ptr<Player> player_         = nullptr;
    std::unique_ptr<GameCamera> gameCamera_ = nullptr;
    std::unique_ptr<TestGround> testGround_ = nullptr;

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