#pragma once
#include "2d/Sprite.h"
#include "Easing/Easing.h"
#include "utility/ParameterEditor/GlobalParameter.h"
#include <memory>
#include <string>

class Player;

class BaseTutorialMission {
public:
    enum class MissionStatus {
        NOT_STARTED,
        IN_PROGRESS,
        COMPLETED,
        FAILED
    };

    enum class ScaleAnimationState {
        NONE,
        APPEARING,
        VISIBLE,
        CLOSING
    };

    struct TutorialEase {
        std::unique_ptr<Easing<float>> xScale;
        std::unique_ptr<Easing<float>> yScale;
        bool isNext_;
        Vector2 tempScale_;
    };

protected:
    enum class StepStatus {
        RUNNING,
        COMPLETED,
        FAILED
    };

public:
    BaseTutorialMission()          = default;
    virtual ~BaseTutorialMission() = default;

    void EasingInit();

    // 基本ライフサイクル
    virtual void Init(const std::string& name, const std::string& spriteName);
    virtual void Update();
    virtual void EndUpdate();
    virtual void SpriteDraw();

    // ミッション制御
    virtual void StartMission();
    virtual void CompleteMission();

    // 各派生クラスで実装する純仮想関数
    virtual void OnMissionStart()    = 0;
    virtual void OnMissionUpdate()   = 0;
    virtual void OnMissionComplete() = 0;

    // ユーティリティ関数
    bool ProcessStep(float limitTime, bool enableEnemySpawn = false);

    // Editor
    virtual void AdjustParam();
    virtual void BindParams();
    virtual void AdjustUniqueParam() = 0;

    // プレイヤー設定
    void SetPlayer(Player* player) { pPlayer_ = player; }

   

protected:
    // 派生クラスで使用する共通機能
    void SetProgress(float progress) { progress_ = std::clamp(progress, 0.0f, 1.0f); }

    void SetGroupName(const std::string& name) { groupName_ = name; }

    // スケール演出制御
    void StartScaleAppearAnimation();
    void StartScaleCloseAnimation();
    void UpdateScaleAnimation();

    // ゲージ関連の新機能
    void InitializeGauge();
    void UpdateGauge();
    void DrawGauge();

protected:
    GlobalParameter* globalParameter_ = nullptr;
    std::string groupName_;
  
    const std::string filePath_ = "Resources/Texture/TutorialMission/";

    Vector2 spritePos_;

    // ゲージ関連の新しいメンバ変数
    Vector2 gaugePos_;
    Vector2 gaugeSize_;
    bool showGauge_;

    std::unique_ptr<Sprite> okSprite_ = nullptr;
    std::unique_ptr<Sprite> gaugeBackSprite_ = nullptr;
    std::unique_ptr<Sprite> gaugeFillSprite_ = nullptr;

    TutorialEase appearEase_;
    TutorialEase closeEase_;
   
   Vector2 tempScale_;
    ScaleAnimationState scaleAnimState_ = ScaleAnimationState::NONE;

    // プレイヤー参照
    Player* pPlayer_                = nullptr;
    std::unique_ptr<Sprite> sprite_ = nullptr;

    // 時間管理
    float currentTime_ = 0.0f;
    float totalTime_   = 0.0f;
    

    // ステータス
    MissionStatus status_  = MissionStatus::NOT_STARTED;
    bool isAbleEnemySpawn_ = false;
    float progress_        = 0.0f; // 0.0f～1.0f

public:
    MissionStatus GetStatus() const { return status_; }
    bool IsCompleted() const { return status_ == MissionStatus::COMPLETED; }
    bool IsFailed() const { return status_ == MissionStatus::FAILED; }
    bool IsInProgress() const { return status_ == MissionStatus::IN_PROGRESS; }
    bool GetIsAbleEnemySpawn() const { return isAbleEnemySpawn_; }

    float GetProgress() const { return progress_; }
    const Vector2& GetCurrentScale() const { return tempScale_; }
    bool IsScaleAnimationFinished() const;

    // 新しく追加するゲッター
    ScaleAnimationState GetScaleAnimationState() const { return scaleAnimState_; }
};