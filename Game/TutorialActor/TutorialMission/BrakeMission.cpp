#include "BrakeMission.h"
#include "Actor/player/Player.h"
#include "Frame/Frame.h"
#include"Actor/Player/Behavior/PlayerBrake.h"
#include <algorithm>
#include <cmath>
#include <imgui.h>

// ========================================
// MovementTutorialMission 実装
// ========================================
BrakeMission::BrakeMission() {
    // デフォルト値を設定
    maxWaitTime_       = 3.0f;
    inputThreshold_    = 0.1f;
    requiredInputTime_ = 2.0f;
}

void BrakeMission::OnMissionStart() {
    currentStep_      = MovementStep::EXPLAIN;
    waitTime_         = 0.0f;
    hasPlayerMoved_   = false;
    currentInputTime_ = 0.0f;
    isInputActive_    = false;

    // ゲージは説明段階では表示しない
    showGauge_ = false;
}

void BrakeMission::OnMissionUpdate() {
    float deltaTime = Frame::DeltaTime();

    switch (currentStep_) {
    case MovementStep::EXPLAIN:
        if (ProcessStep(maxWaitTime_)) {
            currentStep_ = MovementStep::WAIT_INPUT;
            showGauge_   = true; // 入力待ち段階でゲージを表示
            SetProgress(0.0f); 
        }
        break;

    case MovementStep::WAIT_INPUT:
        if (pPlayer_) {
            // 入力チェック
        
             bool inputConditionMet = dynamic_cast<PlayerBrake*>(pPlayer_->GetSpeedBehavior());

             if (inputConditionMet) {
                // 入力条件を満たしている場合
                if (!isInputActive_) {
                    isInputActive_ = true;
                }

                // 入力時間を蓄積
                currentInputTime_ += deltaTime;

                // プログレスを更新（0.0f ～ 1.0f）
                float inputProgress   = std::clamp(currentInputTime_ / requiredInputTime_, 0.0f, 1.0f);
                float currentProgress = (inputProgress * 1.0f); // 0.0f～1.0f
                SetProgress(currentProgress);

                // 必要時間に達したらコンプリート
                if (currentProgress >= 0.95f) {
                    hasPlayerMoved_ = true;
                    currentStep_    = MovementStep::SUCCESS;
                }
            } else {
                // 入力条件を満たしていない場合
                if (isInputActive_) {
                    isInputActive_ = false;
                }

                // 入力時間を徐々に減少
                if (currentInputTime_ > 0.0f) {
                    currentInputTime_ -= deltaTime * 0.5f; // 減少速度は入力蓄積の半分
                    currentInputTime_ = (std::max)(currentInputTime_, 0.0f);

                    // プログレスも更新（0.2f ～ 1.0f）
                    float inputProgress   = std::clamp(currentInputTime_ / requiredInputTime_, 0.0f, 1.0f);
                    float currentProgress = (inputProgress * 1.0f); // 0.0f～1.0f
                    SetProgress(currentProgress);
                }
            }
        }
        break;

    case MovementStep::SUCCESS:
        SetProgress(1.0f);
        showGauge_ = false; // 成功時はゲージを非表示
        CompleteMission();
        break;
    }
}

void BrakeMission::OnMissionComplete() {
    // ミッション完了時の処理
    showGauge_ = false;
}

void BrakeMission::BindParams() {
    BaseTutorialMission::BindParams();
    globalParameter_->Bind(groupName_, "maxWaitTime", &maxWaitTime_);
    globalParameter_->Bind(groupName_, "inputThreshold", &inputThreshold_);
    globalParameter_->Bind(groupName_, "requiredInputTime", &requiredInputTime_);
}

void BrakeMission::AdjustParam() {
    BaseTutorialMission::AdjustParam();
}

void BrakeMission::AdjustUniqueParam() {
#ifdef _DEBUG
    // 基本パラメータ
    ImGui::DragFloat("Max Wait Time", &maxWaitTime_, 0.1f, 1.0f, 10.0f);
    ImGui::DragFloat("Input Threshold", &inputThreshold_, 0.01f, 0.01f, 1.0f);
    ImGui::DragFloat("Required Input Time", &requiredInputTime_, 0.1f, 1.0f, 10.0f);

    // 現在の状態表示
    ImGui::Text("Current Step: %s",
        currentStep_ == MovementStep::EXPLAIN      ? "Explain"
        : currentStep_ == MovementStep::WAIT_INPUT ? "Wait Input"
                                                   : "Success");

    ImGui::Text("Input Active: %s", isInputActive_ ? "Yes" : "No");
    ImGui::Text("Current Input Time: %.2f / %.2f", currentInputTime_, requiredInputTime_);

    if (pPlayer_) {
        float inputValue = std::abs(pPlayer_->GetAngleInput().x);
        ImGui::Text("Current Input Value: %.3f (Threshold: %.3f)", inputValue, inputThreshold_);
    }

    // プログレスバー
    float inputProgress = requiredInputTime_ > 0.0f ? (currentInputTime_ / requiredInputTime_) : 0.0f;
    ImGui::ProgressBar(inputProgress, ImVec2(-1.0f, 0.0f),
        (std::to_string(static_cast<int>(inputProgress * 100)) + "%").c_str());

    // リセットボタン
    if (ImGui::Button("Reset Input Progress")) {
        currentInputTime_ = 0.0f;
        isInputActive_    = false;
        if (currentStep_ == MovementStep::WAIT_INPUT) {
            SetProgress(0.2f);
        }
    }
#endif
}