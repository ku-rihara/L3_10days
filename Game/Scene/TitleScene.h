#pragma once

/// std
#include <string>
#include <array>
#include <memory>

/// engine
#include "BaseScene.h"
#include "utility/ParticleEditor/ParticleEmitter.h"
#include "BaseObject/BaseObject.h"
#include "TitleActor/TitleSprite/TitleSprite.h"

#include "Actor/GameCamera/CameraRendition.h"
#include "Fade/Fade.h"

/// <summary>
/// タイトルシーン
/// </summary>
class TitleScene :public BaseScene {
private:

	using Object = std::unique_ptr<BaseObject>;
	std::vector<Object> object3ds_;

	std::unique_ptr<TitleSprite> titleSprite_ = nullptr;

	std::unique_ptr<Fade> fade_ = nullptr;

	int bgmId_;

public:
	///========================================================
	/// Construct Destruct
	///========================================================
	TitleScene();
	~TitleScene()override;

	///========================================================
	/// private method
	///========================================================

	/// 初期化、更新、描画
	void Init()override;
	void Update()override;
	void ModelDraw()override;
	void SpriteDraw()override;
	void SkyBoxDraw() override;
	void DrawShadow() override;


	void Debug()override;/// debug
	void ViewProjectionUpdate()override;
	void ViewProssess()override;



};