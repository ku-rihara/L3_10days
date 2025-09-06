#include "LockOn.h"
#include "base/TextureManager.h"
#include "input/Input.h"
#include "Matrix4x4.h"
// function
#include "MathFunction.h"
// class
#include "Frame/Frame.h"
#include <algorithm>
#include <vector>

// 初期化
void LockOn::Init() {
    // スプライトの読み込みと作成
    int TextureHandle = TextureManager::GetInstance()->LoadTexture("Resources/Texture/anchorPoint.png");
    lockOnMark_.reset(Sprite::Create(TextureHandle, Vector2{640, 320}, Vector4(1, 1, 1, 1)));
    lockOnMark_->SetAnchorPoint(Vector2(0.5f, 0.5f));

    kDegreeToRadian_ = ToRadian(180.0f);
    // 最小距離
    minDistance_ = 5.0f;
    // 最大距離
    maxDistance_ = 90.0f;
    // 角度範囲
    angleRange_ = kDegreeToRadian_;
}

void LockOn::Update(const std::vector<ILockable*>& lockableTargets, const ViewProjection& viewProjection, FactionType playerFaction) {
    playerFaction_ = playerFaction;

    // ロックオンを外す
    if (target_) {
        if ((Input::IsTriggerPad(0, XINPUT_GAMEPAD_B))) {
            target_ = nullptr;
        } else if (IsOutOfRange(lockableTargets, viewProjection, playerFaction)) {
            target_ = nullptr;
        }
    } else {
        if ((Input::IsTriggerPad(0, XINPUT_GAMEPAD_B))) {
            // ロックオン対象の検索
            Search(lockableTargets, viewProjection, playerFaction);
        }
    }

    // ロックオン継続
    if (target_) {
        // 敵のロックオン座標取得
        Vector3 positionWorld = target_->GetWorldPosition();
        // ワールド座標からスクリーン座標に変換
        Vector3 positionScreen = ScreenTransform(positionWorld, viewProjection);
        // Vector2に格納
        Vector2 positionScreenV2(positionScreen.x, positionScreen.y);
        // 線形補間の計算
        LerpTimeIncrement(0.1f);
        lockOnMarkPos_ = Lerp(prePos_, positionScreenV2, lerpTime_);

        // スプライトの座標を設定
        lockOnMark_->SetPosition(lockOnMarkPos_);
        spriteRotation_ += Frame::DeltaTime();
        // 回転を設定
        lockOnMark_->transform_.rotate.z = (spriteRotation_);
    }
}

void LockOn::Draw() {
    if (target_) {
        lockOnMark_->Draw();
    }
}

void LockOn::Search(const std::vector<ILockable*>& lockableTargets, const ViewProjection& viewProjection, FactionType playerFaction) {
    Vector3 positionView = {};

    std::vector<std::pair<float, const ILockable*>> targets; // 目標

    // 全ての敵に対して順にロックオンを判定
    for (const auto* target : lockableTargets) {
        if (target && target->IsLockable(playerFaction) && IsTargetRange(*target, viewProjection, positionView)) {
            targets.emplace_back(std::make_pair(positionView.z, target));
        }
    }

    // ロックオン対象をリセット
    target_ = nullptr;
    if (!targets.empty()) {
        // 距離で昇順にソート
        std::sort(targets.begin(), targets.end(),
            [](const auto& pair1, const auto& pair2) {
                return pair1.first < pair2.first;
            });
        // ソートの結果一番近い敵をロックオン対象とする
        target_ = targets.front().second;
        // ロックオン前の座標を記録
        prePos_   = lockOnMark_->GetPosition();
        lerpTime_ = 0.0f; // 線形補間の進行度をリセット
    }
}

bool LockOn::IsOutOfRange(const std::vector<ILockable*>& lockableTargets, const ViewProjection& viewProjection, FactionType playerFaction) {
    Vector3 positionView = {};

    // 現在のターゲットが範囲内にあるかチェック
    if (target_) {
        if (target_->IsLockable(playerFaction) && IsTargetRange(*target_, viewProjection, positionView)) {
            return false; // 範囲内
        }
    }

    return true; // 範囲外
}

bool LockOn::IsTargetRange(const ILockable& target, const ViewProjection& viewProjection, Vector3& positionView) {
    // 敵のロックオン座標を取得
    Vector3 positionWorld = target.GetWorldPosition();
    // ワールド→ビュー座標系
    positionView = TransformMatrix(positionWorld, viewProjection.matView_);
    // 距離条件チェック
    if (minDistance_ <= positionView.z && positionView.z <= maxDistance_) {
        // カメラ前方との角度を計算
        float actTangent = std::atan2(std::sqrt(positionView.x * positionView.x + positionView.y * positionView.y), positionView.z);

        // 角度条件チェック（コーンに収まっているか）
        return (std::fabsf(actTangent) <= std::fabsf(angleRange_));
    }
    return false;
}

Vector3 LockOn::GetTargetPosition() const {
    if (ExistTarget()) {
        return target_->GetWorldPosition();
    }
    return Vector3();
}

void LockOn::OnTargetDestroyed(const ILockable* target) {
    if (target_ == target) {
        target_ = nullptr;
    }
}

// 線形補間タイムインクリメント
void LockOn::LerpTimeIncrement(float incrementTime) {
    lerpTime_ += incrementTime;
    if (lerpTime_ >= 1.0f) {
        lerpTime_ = 1.0f;
    }
}