#include "ShotMissileMission.h"
#include "Actor/Player/Player.h"
#include "Actor/player/Player.h"
#include "Frame/Frame.h"
#include <algorithm>
#include <cmath>
#include <imgui.h>

// ========================================
// MovementTutorialMission 実装
// ========================================
ShotMissileMission::ShotMissileMission() {
    // デフォルト値を設定
    maxWaitTime_       = 3.0f;
    inputThreshold_    = 0.1f;
    requiredInputTime_ = 2.0f;
}

void ShotMissileMission::OnMissionStart() {
    currentStep_      = MovementStep::EXPLAIN;
    waitTime_         = 0.0f;
    hasPlayerMoved_   = false;
    currentInputTime_ = 0.0f;
    isInputActive_    = false;

    // ゲージは説明段階では表示しない
    showGauge_ = false;
}

void ShotMissileMission::OnMissionUpdate() {
    switch (currentStep_) {
    case MovementStep::EXPLAIN:
        if (ProcessStep(maxWaitTime_)) {
            currentStep_ = MovementStep::WAIT_INPUT;
            showGauge_   = true; // 入力待ち段階でゲージを表示
            SetProgress(0.0f);
            currentInputTime_ = 0.0f; // カウント初期化
            isInputActive_    = false;
        }
        break;

    case MovementStep::WAIT_INPUT:
        if (pPlayer_) {
            bool inputConditionMet = pPlayer_->GetBulletShooter()->GetIsMissileInput();

            if (inputConditionMet) {
                if (!isInputActive_) {
                    isInputActive_ = true;

                    // ---- ここで1回押しをカウント ----
                    currentInputTime_ += 1.0f;

                    // プログレス更新（2回押しで100%）
                    float inputProgress = std::clamp(currentInputTime_ / 2.0f, 0.0f, 1.0f);
                    SetProgress(inputProgress);

                    // 2回押したら成功
                    if (currentInputTime_ >= 2.0f) {
                        hasPlayerMoved_ = true;
                        currentStep_    = MovementStep::SUCCESS;
                    }
                }
            } else {
                // ボタンを離したら次の押下を受け付ける
                isInputActive_ = false;
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

void ShotMissileMission::OnMissionComplete() {
    // ミッション完了時の処理
    showGauge_ = false;
}

void ShotMissileMission::BindParams() {
    BaseTutorialMission::BindParams();
    globalParameter_->Bind(groupName_, "maxWaitTime", &maxWaitTime_);
    globalParameter_->Bind(groupName_, "inputThreshold", &inputThreshold_);
    globalParameter_->Bind(groupName_, "requiredInputTime", &requiredInputTime_);
}

void ShotMissileMission::AdjustParam() {
    BaseTutorialMission::AdjustParam();
}

void ShotMissileMission::AdjustUniqueParam() {
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