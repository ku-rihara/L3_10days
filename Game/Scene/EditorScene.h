#pragma once

/// baseScene
#include "BaseScene.h"

#include "Easing/Easing.h"
#include "EasingTestObj/EasingTestObj.h"
#include "utility/EasingCreator/Easingeditor.h"
#include "utility/ParticleEditor/ParticleEmitter.h"
#include <array>
#include <memory>
/// <summary>
/// EditorScene
/// </summary>
class EditorScene : public BaseScene {
private:
    /// Particle追加
    std::array<std::unique_ptr<ParticleEmitter>, 5> testEmitter_;

    std::array<std::unique_ptr<ParticleEmitter>, 3> playerShotEmitter_;
    std::array<std::unique_ptr<ParticleEmitter>, 3> playerMissileEmitter_;

    std::unique_ptr<EasingTestObj> easingTestObject_;
    EasingEditor easingEditor_;

public:
    ///========================================================
    /// Constralt destract
    ///========================================================
    EditorScene();
    ~EditorScene() override;

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