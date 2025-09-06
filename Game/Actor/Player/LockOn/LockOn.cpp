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
    int TextureHandle = TextureManager::GetInstance()->LoadTexture("Resources/Texture/default.png");
    lockOnMark_.reset(Sprite::Create(TextureHandle, Vector2{640, 320}, Vector4(1, 1, 1, 1)));
    lockOnMark_->SetAnchorPoint(Vector2(0.5f, 0.5f));

    // 初期パラメータ設定
    minDistance_ = 5.0f;
    maxDistance_ = 90.0f;
    angleRange_  = ToRadian(180.0f);

    lerpTime_        = 0.0f;
    spriteRotation_  = 0.0f;
    autoSearchTimer_ = 0.0f;
}

void LockOn::Update(const std::vector<LockOnVariant>& targets, const ViewProjection& viewProjection, FactionType playerFaction) {
    float deltaTime = Frame::DeltaTime();
    autoSearchTimer_ += deltaTime;

    // 現在のターゲットの有効性チェック
    if (currentTarget_.has_value()) {
        if (!IsLockable(currentTarget_.value(), playerFaction)) {
            // ターゲットが死亡したり、ロック不可能になった場合
            currentTarget_.reset();
            currentTargetIndex_ = 0;
        } else {
            // 範囲外チェック
            Vector3 positionView;
            if (!IsTargetRange(currentTarget_.value(), viewProjection, positionView)) {
                currentTarget_.reset();
                currentTargetIndex_ = 0;
            }
        }
    }

    // 手動切り替え処理
    HandleTargetSwitching(targets, viewProjection, playerFaction);

    // 自動検索処理
    if (autoSearchEnabled_ && autoSearchTimer_ >= autoSearchInterval_) {
        AutoSearchTarget(targets, viewProjection, playerFaction);
        autoSearchTimer_ = 0.0f;
    }

    // UI更新
    UpdateLockOnUI(viewProjection);
}

void LockOn::HandleTargetSwitching(const std::vector<LockOnVariant>& targets, const ViewProjection& viewProjection, FactionType playerFaction) {
    // ボタン入力の検出
    bool currentSwitchInput = Input::IsTriggerPad(0, XINPUT_GAMEPAD_Y);
    bool switchTriggered    = currentSwitchInput && !prevSwitchInput_;
    prevSwitchInput_        = currentSwitchInput;

    if (!switchTriggered)
        return;

    // 有効なターゲットリストを更新
    validTargets_ = GetValidTargets(targets, viewProjection, playerFaction);
    if (validTargets_.empty()) {
        currentTarget_.reset();
        currentTargetIndex_ = 0;
        return;
    }

    // ソート（距離順または角度順）
    std::vector<std::pair<float, LockOnVariant>> sortedTargets;
    for (const auto& target : validTargets_) {
        Vector3 positionView;
        IsTargetRange(target, viewProjection, positionView);

        float sortValue = (switchMode_ == SwitchMode::Distance) ? positionView.z : std::sqrt(positionView.x * positionView.x + positionView.y * positionView.y);
        sortedTargets.emplace_back(sortValue, target);
    }

    if (switchMode_ == SwitchMode::Distance) {
        SortTargetsByDistance(sortedTargets);
    } else {
        SortTargetsByAngle(sortedTargets);
    }

    // ソート後のターゲットリストを更新
    validTargets_.clear();
    for (const auto& pair : sortedTargets) {
        validTargets_.push_back(pair.second);
    }

    // 現在のターゲットがない場合は最初のターゲットを選択
    if (!currentTarget_.has_value()) {
        currentTargetIndex_ = 0;
        currentTarget_      = validTargets_[0];
        prePos_             = lockOnMark_->GetPosition();
        lerpTime_           = 0.0f;
        return;
    }

    // 現在のターゲットのインデックスを見つける
    auto it = std::find(validTargets_.begin(), validTargets_.end(), currentTarget_.value());
    if (it != validTargets_.end()) {
        currentTargetIndex_ = std::distance(validTargets_.begin(), it);
    } else {
        currentTargetIndex_ = 0;
    }

    // 次のターゲットに切り替え
    currentTargetIndex_ = (currentTargetIndex_ + 1) % validTargets_.size();
    currentTarget_      = validTargets_[currentTargetIndex_];

    // UI切り替えのためのリセット
    prePos_   = lockOnMark_->GetPosition();
    lerpTime_ = 0.0f;
}

void LockOn::AutoSearchTarget(const std::vector<LockOnVariant>& targets, const ViewProjection& viewProjection, FactionType playerFaction) {
    // 有効なターゲットを検索
    auto validTargets = GetValidTargets(targets, viewProjection, playerFaction);

    if (validTargets.empty()) {
        if (currentTarget_.has_value()) {
            currentTarget_.reset();
            currentTargetIndex_ = 0;
        }
        return;
    }

    // 現在ターゲットがない場合、最も近いターゲットを自動選択
    if (!currentTarget_.has_value()) {
        std::vector<std::pair<float, LockOnVariant>> sortedTargets;

        for (const auto& target : validTargets) {
            Vector3 positionView;
            IsTargetRange(target, viewProjection, positionView);
            sortedTargets.emplace_back(positionView.z, target);
        }

        SortTargetsByDistance(sortedTargets);

        if (!sortedTargets.empty()) {
            currentTarget_ = sortedTargets[0].second;
            prePos_        = lockOnMark_->GetPosition();
            lerpTime_      = 0.0f;

            // validTargets_も更新
            validTargets_.clear();
            for (const auto& pair : sortedTargets) {
                validTargets_.push_back(pair.second);
            }
            currentTargetIndex_ = 0;
        }
    }
}

std::vector<LockOn::LockOnVariant> LockOn::GetValidTargets(const std::vector<LockOnVariant>& targets, const ViewProjection& viewProjection, FactionType playerFaction) const {
    std::vector<LockOnVariant> validTargets;
    Vector3 positionView;

    for (const auto& target : targets) {
        if (IsLockable(target, playerFaction) && IsTargetRange(target, viewProjection, positionView)) {
            validTargets.push_back(target);
        }
    }

    return validTargets;
}

void LockOn::UpdateLockOnUI(const ViewProjection& viewProjection) {
    if (!currentTarget_.has_value())
        return;

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

void LockOn::SortTargetsByDistance(std::vector<std::pair<float, LockOnVariant>>& validTargets) const {
    std::sort(validTargets.begin(), validTargets.end(),
        [](const auto& a, const auto& b) {
            return a.first < b.first; // 距離昇順
        });
}

void LockOn::SortTargetsByAngle(std::vector<std::pair<float, LockOnVariant>>& validTargets) const {
    std::sort(validTargets.begin(), validTargets.end(),
        [](const auto& a, const auto& b) {
            return a.first < b.first; // 画面中央からの距離昇順
        });
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
        currentTargetIndex_ = 0;

        // validTargets_からも削除
        auto it = std::find(validTargets_.begin(), validTargets_.end(), obj);
        if (it != validTargets_.end()) {
            validTargets_.erase(it);
        }
    }
}

// 既存のヘルパー関数たちはそのまま...
Vector3 LockOn::GetPosition(const LockOnVariant& target) const {
    return std::visit([](auto&& obj) -> Vector3 {
        return obj->GetWorldPosition();
    },
        target);
}

bool LockOn::IsDead(const LockOnVariant& target) const {
    target;
    return false;
}

FactionType LockOn::GetFaction(const LockOnVariant& target) const {
    target;
    return FactionType::Enemy;
}

bool LockOn::IsLockable(const LockOnVariant& target, FactionType playerFaction) const {
    return !IsDead(target) && GetFaction(target) != playerFaction;
}

bool LockOn::IsTargetRange(const LockOnVariant& target, const ViewProjection& viewProjection, Vector3& positionView)const {
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