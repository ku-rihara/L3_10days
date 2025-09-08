#include "LockOn.h"
// target
#include "Actor/Player/Player.h"
#include"Actor/NPC/EnemyNPC.h"
#include"actor/NPC/BoundaryBreaker/BoundaryBreaker.h"
#include"Actor/Station/Enemy/EnemyStation.h"
//
#include "Actor/NPC/Navigation/RectXZWithGatesConstraint.h"
#include "base/TextureManager.h"
#include "Frame/Frame.h"
#include "input/Input.h"
#include "MathFunction.h"
#include "Matrix4x4.h"
#include <algorithm>
#include <cmath>
#include <imgui.h>

const std::vector<Hole>& LockOn::BoundaryHoleSource::GetHoles() const {
    static const std::vector<Hole> kEmpty;
    return boundary ? boundary->GetHoles() : kEmpty;
}

void LockOn::Init() {

    //* グローバルパラメータ
    globalParameter_ = GlobalParameter::GetInstance();
    globalParameter_->CreateGroup(groupName_, false);
    BindParams();
    globalParameter_->SyncParamForGroup(groupName_);

    // スプライトの読み込みと作成
    int TextureHandle = TextureManager::GetInstance()->LoadTexture("Resources/Texture/UI/Reticle.png");
    lockOnMark_.reset(Sprite::Create(TextureHandle, Vector2{0, 0}, Vector4(1, 1, 1, 1)));
    lockOnMark_->SetAnchorPoint(Vector2(0.5f, 0.5f));

    // moveConstraint
    Boundary* boundary   = Boundary::GetInstance();
    holeSource_.boundary = boundary;
    RectXZ rect{-1500.0f, 1500.0f, -1500.0f, 1500.0f};
    moveConstraint_ = std::make_unique<RectXZWithGatesConstraint>(&holeSource_, rect, 0.01f);

    lerpTime_        = 0.0f;
    spriteRotation_  = 0.0f;
    autoSearchTimer_ = 0.0f;
}

void LockOn::Update(const std::vector<LockOnVariant>& targets, const Player* player, const ViewProjection& viewProjection, FactionType playerFaction) {
    float deltaTime = Frame::DeltaTime();
    autoSearchTimer_ += deltaTime;

    // 現在のターゲットの有効性チェック
    if (currentTarget_.has_value()) {
        if (!IsLockable(currentTarget_.value(), playerFaction)) {
            // ターゲットが死亡したり、ロック不可能になった場合
            currentTarget_.reset();
            currentTargetIndex_ = 0;
        } else {
            // 範囲外チェック（プレイヤー基準）
            Vector3 relativePosition;
            if (!IsTargetRange(currentTarget_.value(), player, relativePosition)) {
                currentTarget_.reset();
                currentTargetIndex_ = 0;
            }
        }
    }

    // 手動切り替え処理
    HandleTargetSwitching(targets, player, playerFaction);

    // 自動検索処理
    if (autoSearchEnabled_ && autoSearchTimer_ >= autoSearchInterval_) {
        AutoSearchTarget(targets, player, playerFaction);
        autoSearchTimer_ = 0.0f;
    }

    // UI更新
    UpdateUI(viewProjection);
}

void LockOn::HandleTargetSwitching(const std::vector<LockOnVariant>& targets, const Player* player, FactionType playerFaction) {
    // ホーミングミサイルがアクティブな間はターゲット変更を無効化
    if (isHomingMissileActive_) {
        return;
    }

    // ボタン入力の検出
    bool currentSwitchInput = Input::IsTriggerPad(0, XINPUT_GAMEPAD_Y);
    bool switchTriggered    = currentSwitchInput && !prevSwitchInput_;
    prevSwitchInput_        = currentSwitchInput;

    if (!switchTriggered)
        return;

    // 有効なターゲットリストを更新
    validTargets_ = GetValidTargets(targets, player, playerFaction);
    if (validTargets_.empty()) {
        currentTarget_.reset();
        currentTargetIndex_ = 0;
        return;
    }

    // ソート（距離順または角度順）
    std::vector<std::pair<float, LockOnVariant>> sortedTargets;

    for (const auto& target : validTargets_) {
        Vector3 relativePosition;
        if (IsTargetRange(target, player, relativePosition)) {
            float sortValue;
            if (switchMode_ == SwitchMode::Distance) {
                sortValue = relativePosition.Length(); // 距離
            } else {
                // プレイヤーの前方ベクトルとの角度
                Vector3 playerForward      = player->GetForwardVector();
                Vector3 toTargetNormalized = relativePosition.Normalize();
                float dot                  = Vector3::Dot(playerForward, toTargetNormalized);
                dot                        = std::clamp(dot, -1.0f, 1.0f);
                sortValue                  = std::acos(dot); // 角度（小さいほど前方）
            }
            sortedTargets.emplace_back(sortValue, target);
        }
    }

    // ソート処理
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

    // 現在のターゲットがない場合
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

void LockOn::AutoSearchTarget(const std::vector<LockOnVariant>& targets, const Player* player, FactionType playerFaction) {
    // 有効なターゲットを検索
    auto validTargets = GetValidTargets(targets, player, playerFaction);

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
            Vector3 relativePosition;
            if (IsTargetRange(target, player, relativePosition)) {
                sortedTargets.emplace_back(relativePosition.Length(), target);
            }
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
std::vector<LockOn::LockOnVariant> LockOn::GetValidTargets(const std::vector<LockOnVariant>& targets, const Player* player, FactionType playerFaction) const {
    std::vector<LockOnVariant> validTargets;
    Vector3 relativePosition;

    for (const auto& target : targets) {
        if (IsLockable(target, playerFaction) && IsTargetRange(target, player, relativePosition)) {
            validTargets.push_back(target);
        }
    }

    return validTargets;
}
void LockOn::UpdateUI(const ViewProjection& viewProjection) {
    if (!currentTarget_.has_value())
        return;

    // ターゲットの座標取得
    Vector3 positionWorld = GetPosition(currentTarget_.value());

    // ワールド座標からスクリーン座標に変換
    Vector3 positionScreen = ScreenTransform(positionWorld, viewProjection);

    // Vector2に格納
    Vector2 positionScreenV2(positionScreen.x, positionScreen.y);

    // スケール
    lockOnMark_->SetScale(spriteScale_);

    // 線形補間の計算
    LerpTimeIncrement(targetChangeSpeed_);
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

Vector3 LockOn::GetPosition(const LockOnVariant& target) const {
    return std::visit([](auto&& obj) -> Vector3 {
        return obj->GetWorldPosition();
    },
        target);
}

bool LockOn::IsLockable(const LockOnVariant& target, FactionType playerFaction) const {
    /* return !IsDead(target) && GetFaction(target) != playerFaction;*/
    target;
    playerFaction;
    return true;
}

bool LockOn::IsTargetRange(const LockOnVariant& target, const Player* player, Vector3& relativePosition) const {
    // プレイヤーの位置と向きを取得
    Vector3 playerPos     = player->GetPosition();
    Vector3 playerForward = player->GetForwardVector();

    // ターゲットの位置を取得
    Vector3 targetPos = GetPosition(target);

    // プレイヤーからターゲットへの相対ベクトル
    Vector3 toTarget = targetPos - playerPos;
    relativePosition = toTarget;

    //=======境界との当たり判定をチェック============//
    Vector3 from = player->GetPosition();
    Vector3 to   = targetPos;

    // FilterMove
    if (moveConstraint_) {
       moveConstraint_->FilterMove(from, to);
    }

    // ブロック判定
    if (RectXZWithGatesConstraint* rectConstraint = dynamic_cast<RectXZWithGatesConstraint*>(moveConstraint_.get())) {
        if (rectConstraint->WasBlocked()) {
            return false;
        }
    }

    // 距離チェック
    float distance = toTarget.Length();
    if (distance < minDistance_ || distance > maxDistance_) {
        return false;
    }

    // プレイヤーの前方ベクトルとの角度チェック
    if (distance > 0.001f) { // ゼロ除算回避
        Vector3 toTargetNormalized = toTarget.Normalize();
        float dot                  = Vector3::Dot(playerForward, toTargetNormalized);

        // dotを-1〜1の範囲にクランプ
        dot         = std::clamp(dot, -1.0f, 1.0f);
        float angle = std::acos(dot);

        // 角度が範囲内かチェック（angleRange_は度単位）
        return angle <= ToRadian(angleRange_);
    }

    return false;
}

void LockOn::LerpTimeIncrement(float incrementTime) {
    lerpTime_ += incrementTime;
    if (lerpTime_ >= 1.0f) {
        lerpTime_ = 1.0f;
    }
}

///=========================================================
/// バインド
///==========================================================
void LockOn::BindParams() {
    globalParameter_->Bind(groupName_, "minDistance", &minDistance_);
    globalParameter_->Bind(groupName_, "maxDistance", &maxDistance_);
    globalParameter_->Bind(groupName_, "angleRange", &angleRange_);
    globalParameter_->Bind(groupName_, "spriteScale", &spriteScale_);
    globalParameter_->Bind(groupName_, "targetChangeSpeed", &targetChangeSpeed_);
}
///=========================================================
/// パラメータ調整
///==========================================================
void LockOn::AdjustParam() {

#ifdef _DEBUG
    if (ImGui::CollapsingHeader(groupName_.c_str())) {
        ImGui::PushID(groupName_.c_str());

        ImGui::DragFloat("minDistance", &minDistance_, 0.1f);
        ImGui::DragFloat("maxDistance", &maxDistance_, 0.1f);
        ImGui::DragFloat("angleRange", &angleRange_, 0.1f);
        ImGui::DragFloat("targetChangeSpeed", &targetChangeSpeed_, 0.1f);
        ImGui::DragFloat2("spriteScale", &spriteScale_.x, 0.1f);

        // セーブ・ロード
        globalParameter_->ParamSaveForImGui(groupName_);
        globalParameter_->ParamLoadForImGui(groupName_);

        ImGui::PopID();
    }
#endif // _DEBUG
}

const LockOn::LockOnVariant* LockOn::GetCurrentTarget() const {
    return currentTarget_ ? &(*currentTarget_) : nullptr;
}

// bool LockOn::IsDead(const LockOnVariant& target) const {
//     target;
//     return false;
// }
//
// FactionType LockOn::GetFaction(const LockOnVariant& target) const {
//     target;
//     return FactionType::Enemy;
// }
