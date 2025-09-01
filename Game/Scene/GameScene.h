#pragma once

/// base
#include "Actor/GameCamera/GameCamera.h"
#include "Actor/Player/Player.h"
#include "BaseScene.h"
#include "SkyBox/SkyBox.h"
#include "Actor/Station/Base/BaseStation.h"

#include<map>

class GameScene : public BaseScene {
private:
	///========================================================
	/// Private variants
	///========================================================

	std::unique_ptr<SkyBox> skuBox_ = nullptr;
	std::unique_ptr<Player> player_ = nullptr;
	std::unique_ptr<GameCamera> gameCamera_ = nullptr;

	std::map<FactionType,std::unique_ptr<BaseStation>> stations_;
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