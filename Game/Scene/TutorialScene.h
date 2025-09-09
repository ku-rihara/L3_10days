#pragma once

/// std
#include <array>
#include <memory>
#include <string>

/// engine
#include "Actor/Boundary/Boundary.h"
#include "Actor/GameCamera/GameCamera.h"
#include "Actor/Player/LockOn/LockOn.h"
#include "Actor/Player/Player.h"
#include "Actor/SkyDome/SkyDome.h"
#include "BaseScene.h"
#include "Fade/Fade.h"
#include "Actor/MiniMap/MiniMap.h"
#include "Actor/UI/GameUIs.h"
#include "Actor/Effects/GameScreenEffect/GameScreenEffect.h"
#include <Details/Faction.h>
#include <Actor/Station/Base/BaseStation.h>
#include <map>

class TutorialScene : public BaseScene {
private:
    std::unique_ptr<Fade> fade_ = nullptr;

    std::unique_ptr<Player> player_         = nullptr;
    std::unique_ptr<GameCamera> gameCamera_ = nullptr;
    std::unique_ptr<LockOn> lockOn_         = nullptr;
    std::unique_ptr<SkyDome> skyDome_       = nullptr;

    std::unique_ptr<MiniMap> miniMap_ = nullptr;
    std::unique_ptr<GameUIs> uis_     = nullptr;
    std::map<FactionType, std::unique_ptr<BaseStation>> stations_;

      /// ----- Effect ----- ///
    std::unique_ptr<GameScreenEffect> outsideWarning_ = nullptr;
    std::unique_ptr<class PlayerEngineEffect> engineEffect_;
    std::unique_ptr<class PlayerLocusEffect> playerLocusEffect_ = nullptr;

    Boundary* boundary_ = nullptr;

public:
    ///========================================================
    /// Construct Destruct
    ///========================================================
    TutorialScene();
    ~TutorialScene() override;

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

    void TutorialUpdate();

    void Debug() override; /// debug
    void ViewProjectionUpdate() override;
    void ViewProssess() override;
};