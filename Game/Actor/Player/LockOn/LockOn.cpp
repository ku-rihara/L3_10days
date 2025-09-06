#include "LockOn.h"
// target
#include "Actor/NPC/BoundaryBreaker/BoundaryBreaker.h"
#include "Actor/NPC/EnemyNPC.h"
//
#include "base/TextureManager.h"
#include "Frame/Frame.h"
#include "input/Input.h"
#include "MathFunction.h"
#include "Matrix4x4.h"
#include <algorithm>
#include <cmath>

void LockOn::Init() {
    // スプライトの読み込みと作成
    //int TextureHandle = TextureManager::GetInstance()->LoadTexture("Resources/Texture/anchorPoint.png");
    //lockOnMark_.reset(Sprite::Create(TextureHandle, Vector2{640, 320}, Vector4(1, 1, 1, 1)));
    //lockOnMark_->SetAnchorPoint(Vector2(0.5f, 0.5f));

    //// 初期パラメータ設定
    //minDistance_ = 5.0f;
    //maxDistance_ = 90.0f;
    //angleRange_  = ToRadian(180.0f);

    //lerpTime_       = 0.0f;
    //spriteRotation_ = 0.0f;
}

void LockOn::Update(const std::vector<LockOnVariant>& targets, const ViewProjection& viewProjection, FactionType playerFaction) {
    // ロックオンを外す処理
    if (currentTarget_.has_value()) {
        if (Input::IsTriggerPad(0, XINPUT_GAMEPAD_B)) {
            currentTarget_.reset();
        } else if (!IsLockable(currentTarget_.value(), playerFaction)) {
            // ターゲットが死亡したり、ロック不可能になった場合
            currentTarget_.reset();
        } else {
            // 範囲外チェック
            Vector3 positionView;
            if (!IsTargetRange(currentTarget_.value(), viewProjection, positionView)) {
                currentTarget_.reset();
            }
        }
    } else {
        // ロックオン対象の検索
        if (Input::IsTriggerPad(0, XINPUT_GAMEPAD_B)) {
            Search(targets, viewProjection, playerFaction);
        }
    }

    // ロックオン継続処理
    if (currentTarget_.has_value()) {
        // ターゲットの座標取得
        Vector3 positionWorld = GetPosition(currentTarget_.value());
        // ワールド座標からスクリーン座標に変換
        Vector3 positionScreen = ScreenTransform(positionWorld, viewProjection);
        // Vector2に格納
        Vector2 positionScreenV2(positionScreen.x, positionScreen.y);

        // 線形補間の計算
        LerpTimeIncrement(0.1f);
        lockOnMarkPos_ = Lerp(prePos_, positionScreenV2, lerpTime_);

        // スプライトの座標と回転を設定
        lockOnMark_->SetPosition(lockOnMarkPos_);
        spriteRotation_ += Frame::DeltaTime();
        lockOnMark_->transform_.rotate.z = spriteRotation_;
    }
}

void LockOn::Draw() {
    if (currentTarget_.has_value()) {
        lockOnMark_->Draw();
    }
}

Vector3 LockOn::GetTargetPosition() const {
    if (currentTarget_.has_value()) {
        return GetPosition(currentTarget_.value());
    }
    return Vector3{};
}

void LockOn::OnObjectDestroyed(const LockOnVariant& obj) {
    if (currentTarget_.has_value() && currentTarget_.value() == obj) {
        currentTarget_.reset();
    }
}

// ヘルパー関数の実装
Vector3 LockOn::GetPosition(const LockOnVariant& target) const {
    return std::visit([](auto&& obj) -> Vector3 {
        return obj->GetWorldPosition();
    },
        target);
}

bool LockOn::IsDead(const LockOnVariant& target) const {
    /*return std::visit([](auto&& obj) -> bool {
        if constexpr (std::is_same_v<std::decay_t<decltype(obj)>, EnemyNPC*>) {
            return obj->GetIsDeath();
        } else if constexpr (std::is_same_v<std::decay_t<decltype(obj)>, BoundaryBreaker*>) {
            return obj->GetIsDeath();
        }
        return true;
    },
        target);*/
    target;
    return false;
}

FactionType LockOn::GetFaction(const LockOnVariant& target) const {
    /*return std::visit([](auto&& obj) -> FactionType {
        return obj->GetFaction();
    },
        target);*/
    target;
    return FactionType::Enemy;
}

bool LockOn::IsLockable(const LockOnVariant& target, FactionType playerFaction) const {
    return !IsDead(target) && GetFaction(target) != playerFaction;
}

void LockOn::Search(const std::vector<LockOnVariant>& targets, const ViewProjection& viewProjection, FactionType playerFaction) {
    Vector3 positionView = {};
    std::vector<std::pair<float, LockOnVariant>> validTargets;

    // 全てのターゲットに対してロックオン判定
    for (const auto& target : targets) {
        if (IsLockable(target, playerFaction) && IsTargetRange(target, viewProjection, positionView)) {
            validTargets.emplace_back(positionView.z, target);
        }
    }

    // ロックオン対象をリセット
    currentTarget_.reset();

    if (!validTargets.empty()) {
        // 距離で昇順にソート
        std::sort(validTargets.begin(), validTargets.end(),
            [](const auto& a, const auto& b) {
                return a.first < b.first;
            });

        // 最も近い敵をロックオン対象とする
        currentTarget_ = validTargets.front().second;

        // ロックオン前の座標を記録
        prePos_   = lockOnMark_->GetPosition();
        lerpTime_ = 0.0f; // 線形補間の進行度をリセット
    }
}

bool LockOn::IsTargetRange(const LockOnVariant& target, const ViewProjection& viewProjection, Vector3& positionView) {
    // ターゲットの座標を取得
    Vector3 positionWorld = GetPosition(target);

    // ワールド→ビュー座標系
    positionView = TransformMatrix(positionWorld, viewProjection.matView_);

    // 距離条件チェック
    if (minDistance_ <= positionView.z && positionView.z <= maxDistance_) {
        // カメラ前方との角度を計算
        float actTangent = std::atan2(
            std::sqrt(positionView.x * positionView.x + positionView.y * positionView.y),
            positionView.z);

        // 角度条件チェック（コーンに収まっているか）
        return (std::abs(actTangent) <= std::abs(angleRange_));
    }

    return false;
}

void LockOn::LerpTimeIncrement(float incrementTime) {
    lerpTime_ += incrementTime;
    if (lerpTime_ >= 1.0f) {
        lerpTime_ = 1.0f;
    }
}