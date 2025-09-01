#pragma once

/// base
#include "BaseScene.h"
#include "SkyBox/SkyBox.h"
#include"Actor/Player/Player.h"

class GameScene : public BaseScene {
private:
    ///========================================================
    /// Private variants
    ///========================================================

    std::unique_ptr<SkyBox> skuBox_ = nullptr;
    std::unique_ptr<Player> player_ = nullptr;

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