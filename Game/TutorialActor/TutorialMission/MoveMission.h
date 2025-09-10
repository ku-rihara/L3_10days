#pragma once
#include "BaseTutorialMission.h"

// ========================================
// 移動チュートリアル
// ========================================
class MoveMission : public BaseTutorialMission {
public:
    enum class MovementStep {
        EXPLAIN,
        WAIT_INPUT,
        SUCCESS
    };

public:
    MoveMission();
    ~MoveMission() = default;

    void OnMissionStart() override;
    void OnMissionUpdate() override;
    void OnMissionComplete() override;
    void BindParams() override;
    void AdjustParam() override;
    void AdjustUniqueParam() override;

private:
    MovementStep currentStep_;
    float waitTime_;
    float maxWaitTime_;
    bool hasPlayerMoved_;

    // 移動入力判定用パラメータ
    float inputThreshold_; // 入力判定の閾値
    float requiredInputTime_; // 必要な入力維持時間
    float currentInputTime_; // 現在の入力維持時間
    bool isInputActive_; // 現在入力が有効かどうか

    // プレイヤーの初期位置
    float initialPlayerX_, initialPlayerY_;
};