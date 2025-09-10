#pragma once
#include "BaseTutorialMission.h"
#include <array>
#include <functional>
#include <memory>
#include <vector>

class Player;

class TutorialMissionManager {
public:
    using MissionCompleteCallback = std::function<void(int missionIndex)>;

    enum class TutorialStatus {
        NOT_STARTED,
        IN_PROGRESS,
        COMPLETED,
        PAUSED,
        TRANSITIONING // ミッション間の遷移状態
    };

    enum class TransitionState {
        NONE,
        WAITING_FOR_CLOSE, // 前のミッションの終了アニメーション待ち
        DELAY_BETWEEN_MISSIONS, // ミッション間の遅延
        STARTING_NEXT // 次のミッションの開始
    };

public:
    TutorialMissionManager();
    ~TutorialMissionManager() = default;

    // 基本ライフサイクル
    void Init();
    void Update();
    void EndUpdate();
    void SpriteDraw();

    // ミッション管理
    void AddMission(std::unique_ptr<BaseTutorialMission> mission);
    void StartTutorial();
    void PauseTutorial();
    void ResumeTutorial();
    void SkipCurrentMission();

    // プレイヤー設定
    void SetPlayer(Player* player);

    // コールバック設定
    void SetMissionCompleteCallback(MissionCompleteCallback callback) {
        missionCompleteCallback_ = callback;
    }

    // Editor用
    void AdjustParam();

private:
    void CheckMissionCompletion();
    void MoveToNextMission();
    void UpdateTransition();
    void StartMissionTransition();

private:
    std::array<std::unique_ptr<BaseTutorialMission>,8> missions_;
    int currentMissionIndex_;
    TutorialStatus status_;
    Player* pPlayer_ = nullptr;

    // 遷移管理
    TransitionState transitionState_;
    float transitionTimer_;
    float missionTransitionDelay_; // ミッション間の遅延時間
    bool isWaitingForCloseAnimation_ = false;

    MissionCompleteCallback missionCompleteCallback_;

    // 統計情報
    float totalTutorialTime_;
    int completedMissions_;

public:
    // ゲッター
    TutorialStatus GetStatus() const { return status_; }
    int GetCurrentMissionIndex() const { return currentMissionIndex_; }
    int GetTotalMissions() const { return static_cast<int>(missions_.size()); }
    BaseTutorialMission* GetCurrentMission() const;
    bool IsCompleted() const { return status_ == TutorialStatus::COMPLETED; }
    bool IsInProgress() const { return status_ == TutorialStatus::IN_PROGRESS; }
    bool IsTransitioning() const { return status_ == TutorialStatus::TRANSITIONING; }
    float GetTotalProgress() const;
};