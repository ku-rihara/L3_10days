#include "TutorialMissionManager.h"
#include "Frame/Frame.h"

//
#include "TutorialActor/TutorialMission/BrakeMission.h"
#include "TutorialActor/TutorialMission/BurstMission.h"
#include "TutorialActor/TutorialMission/MoveMission.h"
#include "TutorialActor/TutorialMission/MoveMissionY.h"
#include "TutorialActor/TutorialMission/ShotBulletMission.h"
#include "TutorialActor/TutorialMission/ShotMissileMission.h"
#include <imgui.h>

TutorialMissionManager::TutorialMissionManager() {
    missionTransitionDelay_ = 0.5f; // デフォルト2秒の遅延
    transitionState_        = TransitionState::NONE;
    transitionTimer_        = 0.0f;
}

void TutorialMissionManager::Init() {
    currentMissionIndex_ = -1;
    status_              = TutorialStatus::NOT_STARTED;
    totalTutorialTime_   = 0.0f;
    completedMissions_   = 0;
    transitionState_     = TransitionState::NONE;
    transitionTimer_     = 0.0f;

    // ミッション初期化
    //
    // Pitch,Roll
    missions_[0] = std::make_unique<MoveMission>();
    missions_[0]->Init("TutorialMissionMoveX", "MoveXMIssion");

    // Yaw
    missions_[1] = std::make_unique<MoveMissionY>();
    missions_[1]->Init("TutorialMissionMoveY", "MoveYMission");

    // Burst
    missions_[2] = std::make_unique<BurstMission>();
    missions_[2]->Init("TutorialMissionBurst", "BurstMission");

    // Brake
    missions_[3] = std::make_unique<BrakeMission>();
    missions_[3]->Init("TutorialMissionBrake", "BrakeMission");

    // BulletShot
    missions_[4] = std::make_unique<ShotBulletMission>();
    missions_[4]->Init("TutorialMissionBulletShot", "BulletShotMission");

    // ShotMissileMission
    missions_[5] = std::make_unique<ShotMissileMission>();
    missions_[5]->Init("TutorialMissionShotMissile", "ShotMissile");

    // 全ミッションを初期化
    for (auto& mission : missions_) {
        if (mission && pPlayer_) {
            mission->SetPlayer(pPlayer_);
        }
    }
}

void TutorialMissionManager::Update() {
    if (status_ != TutorialStatus::IN_PROGRESS && status_ != TutorialStatus::TRANSITIONING) {
        return;
    }

    totalTutorialTime_ += Frame::DeltaTime();

    // 遷移状態の更新
    if (status_ == TutorialStatus::TRANSITIONING) {
        UpdateTransition();

        // 遷移中でも現在のミッションのアニメーションは更新する必要がある
        BaseTutorialMission* currentMission = GetCurrentMission();
        if (currentMission) {
            currentMission->Update(); // これによりクローズアニメーションが更新される
        }
        return;
    }

    // 現在のミッションを更新
    BaseTutorialMission* currentMission = GetCurrentMission();
    if (currentMission) {
        currentMission->Update();
        CheckMissionCompletion();
    }
}

void TutorialMissionManager::UpdateTransition() {
    transitionTimer_ += Frame::DeltaTime();

    switch (transitionState_) {
    case TransitionState::WAITING_FOR_CLOSE: {

        BaseTutorialMission* currentMission = GetCurrentMission();
        if (currentMission) {
            bool isCompleted    = currentMission->IsCompleted();
            bool isAnimFinished = currentMission->IsScaleAnimationFinished();
            auto animState      = currentMission->GetScaleAnimationState();

            // ミッションが完了済みでクローズアニメーションが終了している場合
            if (isCompleted && isAnimFinished && animState == BaseTutorialMission::ScaleAnimationState::NONE) {
                transitionState_ = TransitionState::DELAY_BETWEEN_MISSIONS;
                transitionTimer_ = 0.0f;
            }
        } else {
            // 現在のミッションが存在しない場合は即座に次へ
            transitionState_ = TransitionState::DELAY_BETWEEN_MISSIONS;
            transitionTimer_ = 0.0f;
        }
        break;
    }

    case TransitionState::DELAY_BETWEEN_MISSIONS:
        // 指定時間待機
        if (transitionTimer_ >= missionTransitionDelay_) {
            transitionState_ = TransitionState::STARTING_NEXT;
            transitionTimer_ = 0.0f;
        }
        break;

    case TransitionState::STARTING_NEXT:
        // 次のミッションに移行
        MoveToNextMission();
        status_          = TutorialStatus::IN_PROGRESS;
        transitionState_ = TransitionState::NONE;
        break;

    case TransitionState::NONE:
    default:
        // 何もしない
        break;
    }
}
void TutorialMissionManager::EndUpdate() {
    BaseTutorialMission* currentMission = GetCurrentMission();
    if (currentMission) {
        currentMission->EndUpdate();
    }
}

void TutorialMissionManager::SpriteDraw() {
    BaseTutorialMission* currentMission = GetCurrentMission();
    if (currentMission) {
        currentMission->SpriteDraw();
    }
}

void TutorialMissionManager::AddMission(std::unique_ptr<BaseTutorialMission> mission) {
    // 空いているスロットを探して追加
    for (size_t i = 0; i < missions_.size(); ++i) {
        if (!missions_[i]) {
            missions_[i] = std::move(mission);
            if (pPlayer_) {
                missions_[i]->SetPlayer(pPlayer_);
            }
            break;
        }
    }
}

void TutorialMissionManager::SetPlayer(Player* player) {
    pPlayer_ = player;

    // 既存の全ミッションにプレイヤーを設定
    for (auto& mission : missions_) {
        if (mission) {
            mission->SetPlayer(pPlayer_);
        }
    }
}

void TutorialMissionManager::StartTutorial() {
    if (missions_.empty() || status_ == TutorialStatus::IN_PROGRESS) {
        return;
    }

    status_                     = TutorialStatus::IN_PROGRESS;
    currentMissionIndex_        = 0;
    totalTutorialTime_          = 0.0f;
    completedMissions_          = 0;
    transitionState_            = TransitionState::NONE;
    transitionTimer_            = 0.0f;
    isWaitingForCloseAnimation_ = false; // フラグもリセット

    // 最初のミッションを開始
    BaseTutorialMission* firstMission = GetCurrentMission();
    if (firstMission) {
        firstMission->StartMission();
    }
}

void TutorialMissionManager::StartMissionTransition() {
    status_          = TutorialStatus::TRANSITIONING;
    transitionState_ = TransitionState::WAITING_FOR_CLOSE;
    transitionTimer_ = 0.0f;
}

void TutorialMissionManager::PauseTutorial() {
    if (status_ == TutorialStatus::IN_PROGRESS) {
        status_ = TutorialStatus::PAUSED;
    }
}

void TutorialMissionManager::ResumeTutorial() {
    if (status_ == TutorialStatus::PAUSED) {
        status_ = TutorialStatus::IN_PROGRESS;
    }
}

void TutorialMissionManager::SkipCurrentMission() {
    BaseTutorialMission* currentMission = GetCurrentMission();
    if (currentMission && currentMission->IsInProgress()) {
        currentMission->CompleteMission();
    }
}

BaseTutorialMission* TutorialMissionManager::GetCurrentMission() const {
    if (currentMissionIndex_ >= 0 && currentMissionIndex_ < static_cast<int>(missions_.size())) {
        return missions_[currentMissionIndex_].get();
    }
    return nullptr;
}

float TutorialMissionManager::GetTotalProgress() const {
    if (missions_.empty()) {
        return 0.0f;
    }

    // アクティブなミッション数を計算
    int activeMissions = 0;
    for (const auto& mission : missions_) {
        if (mission) {
            activeMissions++;
        }
    }

    if (activeMissions == 0) {
        return 0.0f;
    }

    float totalProgress = static_cast<float>(completedMissions_);

    // 現在のミッションの進度も加算
    BaseTutorialMission* currentMission = GetCurrentMission();
    if (currentMission && currentMission->IsInProgress()) {
        totalProgress += currentMission->GetProgress();
    }

    return totalProgress / static_cast<float>(activeMissions);
}

void TutorialMissionManager::CheckMissionCompletion() {
    BaseTutorialMission* currentMission = GetCurrentMission();
    if (!currentMission) {
        return;
    }

    // ミッションが完了した瞬間の処理（一度だけ実行）
    if (currentMission->IsCompleted() && status_ == TutorialStatus::IN_PROGRESS && !isWaitingForCloseAnimation_) {

        completedMissions_++;

        // コールバック呼び出し
        if (missionCompleteCallback_) {
            missionCompleteCallback_(currentMissionIndex_);
        }

        // クローズアニメーション待機フラグを設定
        isWaitingForCloseAnimation_ = true;

        // ここでクローズアニメーションが自動的に開始される（CompleteMission()内で）
    }

    // クローズアニメーション待機中で、アニメーションが終了したら遷移開始
    if (isWaitingForCloseAnimation_ && currentMission->IsCompleted()) {
        auto animState      = currentMission->GetScaleAnimationState();
        bool isAnimFinished = currentMission->IsScaleAnimationFinished();

        // クローズアニメーションが完了したら遷移開始
        if (animState == BaseTutorialMission::ScaleAnimationState::NONE && isAnimFinished) {
            StartMissionTransition();
            isWaitingForCloseAnimation_ = false;
        }
        // または、クローズアニメーション中でも終了判定が取れている場合
        else if (animState == BaseTutorialMission::ScaleAnimationState::CLOSING && isAnimFinished) {
            StartMissionTransition();
            isWaitingForCloseAnimation_ = false;
        }
    }
}

void TutorialMissionManager::MoveToNextMission() {
    currentMissionIndex_++;

    // 有効な次のミッションを探す
    while (currentMissionIndex_ < static_cast<int>(missions_.size()) && !missions_[currentMissionIndex_]) {
        currentMissionIndex_++;
    }

    if (currentMissionIndex_ >= static_cast<int>(missions_.size())) {
        // 全ミッション完了
        status_              = TutorialStatus::COMPLETED;
        currentMissionIndex_ = -1;
    } else {
        // 次のミッションを開始（ここでアピアアニメーションが自動的に開始される）
        BaseTutorialMission* nextMission = GetCurrentMission();
        if (nextMission) {
            nextMission->StartMission();
        }
    }
}
void TutorialMissionManager::AdjustParam() {
#ifdef _DEBUG
    if (ImGui::CollapsingHeader("Tutorial Manager")) {
        ImGui::Text("Status: %s",
            status_ == TutorialStatus::NOT_STARTED     ? "Not Started"
            : status_ == TutorialStatus::IN_PROGRESS   ? "In Progress"
            : status_ == TutorialStatus::COMPLETED     ? "Completed"
            : status_ == TutorialStatus::TRANSITIONING ? "Transitioning"
                                                       : "Paused");

        ImGui::Text("Transition State: %s",
            transitionState_ == TransitionState::NONE                     ? "None"
            : transitionState_ == TransitionState::WAITING_FOR_CLOSE      ? "Waiting for Close"
            : transitionState_ == TransitionState::DELAY_BETWEEN_MISSIONS ? "Delay Between Missions"
                                                                          : "Starting Next");

        ImGui::Text("Current Mission: %d / %d",
            currentMissionIndex_ + 1, static_cast<int>(missions_.size()));

        ImGui::ProgressBar(GetTotalProgress(), ImVec2(-1.0f, 0.0f),
            (std::to_string(static_cast<int>(GetTotalProgress() * 100)) + "%").c_str());

        ImGui::Text("Total Time: %.2f", totalTutorialTime_);
        ImGui::Text("Completed Missions: %d", completedMissions_);
        ImGui::Text("Player Set: %s", pPlayer_ ? "Yes" : "No");
        ImGui::Text("Transition Timer: %.2f", transitionTimer_);

        ImGui::DragFloat("Mission Transition Delay", &missionTransitionDelay_, 0.1f, 0.1f, 10.0f);

        if (ImGui::Button("Start Tutorial")) {
            StartTutorial();
        }

        ImGui::SameLine();
        if (ImGui::Button("Skip Current")) {
            SkipCurrentMission();
        }

        // 現在のミッションのパラメータ表示
        BaseTutorialMission* currentMission = GetCurrentMission();
        if (currentMission) {
            ImGui::Separator();
            ImGui::Text("Current Mission Parameters:");
            currentMission->AdjustParam();
        }
    }
#endif // _DEBUG
}