#pragma once

#include<map>

/// base
#include "Actor/GameCamera/GameCamera.h"
#include "Actor/Player/Player.h"
#include "BaseScene.h"
#include "SkyBox/SkyBox.h"

// game
#include "Actor/Station/Base/BaseStation.h"
#include "DebugGround/TestGround.h"
#include "Actor/Boundary/Boundary.h"
#include "Actor/MiniMap/MiniMap.h"
#include "Actor/SkyDome/SkyDome.h"
#include "Actor/Station/UnitDirector/QuotaUnitDirector.h"
#include "Actor/UI/GameUIs.h"
#include "Actor/Effects/PlayerOutsideWarning/PlayerOutsideWarning.h"
#include "Actor/Spline/Spline.h"

class GameScene : public BaseScene {
private:
	///========================================================
	/// Private variants
	///========================================================

	std::unique_ptr<SkyDome> skyDome_ = nullptr;
	std::unique_ptr<Player> player_ = nullptr;
	std::unique_ptr<GameCamera> gameCamera_ = nullptr;
	 std::unique_ptr<TestGround> testGround_     = nullptr;
	/// game objects
	Boundary* boundary_ = nullptr;

	/// ----- UI ----- ///
	std::unique_ptr<MiniMap> miniMap_ = nullptr;
	std::unique_ptr<GameUIs> uis_ = nullptr;

	/// ----- Effect ----- ///
	std::unique_ptr<PlayerOutsideWarning> outsideWarning_ = nullptr;

	std::map<FactionType,std::unique_ptr<BaseStation>> stations_;
	std::unique_ptr<QuotaUnitDirector> director_;


	/// debug
	Spline spline_;

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