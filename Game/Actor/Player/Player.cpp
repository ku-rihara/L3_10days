#include "Player.h"
#include "Actor/Boundary/Boundary.h"
#include "Actor/NPC/Navigation/RectXZWithGatesConstraint.h"
#include "Frame/Frame.h"
#include "input/Input.h"
#include "MathFunction.h"

// behavior
#include "Behavior/PlayerAcceleUnattended.h"
#include <imgui.h>
#include <numbers>

const std::vector<Hole>& Player::BoundaryHoleSource::GetHoles() const {
    static const std::vector<Hole> kEmpty;
    return boundary ? boundary->GetHoles() : kEmpty;
}

void Player::Init() {

    // グローバルパラメータ
    globalParameter_ = GlobalParameter::GetInstance();
    globalParameter_->CreateGroup(groupName_, false);
    BindParams();
    globalParameter_->SyncParamForGroup(groupName_);

    // モデル作成
    obj3d_.reset(Object3d::CreateModel("Player.obj"));
    obj3d_->transform_.rotateOder_ = RotateOder::Quaternion;

    // transform初期化
    baseTransform_.Init();
    baseTransform_.rotateOder_ = RotateOder::Quaternion;
    baseTransform_.quaternion_ = Quaternion::Identity();
    obj3d_->transform_.parent_ = &baseTransform_;

    // レティクル
    reticle_ = std::make_unique<PlayerReticle>();
    reticle_->Init();

    // 弾初期化
    bulletShooter_ = std::make_unique<PlayerBulletShooter>();
    bulletShooter_->Init();

    // moveConstraint
    Boundary* boundary   = Boundary::GetInstance();
    holeSource_.boundary = boundary;
    RectXZ rect{-1500.0f, 1500.0f, -1500.0f, 1500.0f};
    moveConstraint_ = std::make_unique<RectXZWithGatesConstraint>(&holeSource_, rect, 0.01f);

    // Speed Init
    SpeedInit();

    ChangeSpeedBehavior(std::make_unique<PlayerAccelUnattended>(this));
}

void Player::Update() {
    // 入力処理
    HandleInput();

    // スピード
    UpdateSpeedBehavior();
    speedBehavior_->Update();

    // 回転更新
    RotateUpdate();

    // 移動更新
    MoveUpdate();

    // レティクル
    reticle_->Update(this, viewProjection_);

    // 弾丸システム更新
    if (bulletShooter_) {
        bulletShooter_->Update(this);
    }

    // トランスフォーム更新
    BaseObject::Update();
}

void Player::MoveUpdate() {

    float deltaTime = Frame::DeltaTime();

    ReboundByBoundary();

    // 前方速度
    Vector3 forwardVelocity = GetForwardVector() * speedParam_.currentForwardSpeed * deltaTime;
    velocity_               = forwardVelocity;

    // 跳ね返りの分を加えて適応
    baseTransform_.translation_ += velocity_ + reboundVelocity_;
}

void Player::HandleInput() {
    Input* input = Input::GetInstance();

    // 入力値をリセット
    Vector2 stickL      = Vector2::ZeroVector();
    float pawInputValue = 0.0f;

    // ゲームパッドの入力
    stickL = Input::GetPadStick(0, 0);

    // キーボード入力
    if (stickL.Length() < 0.1f) {

        if (input->PushKey(DIK_W)) {
            stickL.y = 1.0f;
        }
        if (input->PushKey(DIK_S)) {
            stickL.y = -1.0f;
        }
        if (input->PushKey(DIK_A)) {
            stickL.x = -1.0f;
        }
        if (input->PushKey(DIK_D)) {
            stickL.x = 1.0f;
        }
    }

    // 自動処理による入力
    if (isAutoRotateByCollision) {
        stickL.y = autoRotateDirection_;
    }

    // コントローラ処理
    if (stickL.Length() > 0.1f) {
        stickL = stickL.Normalize() * std::min(stickL.Length(), 1.0f);
    } else {
        stickL = {0.0f, 0.0f};
    }

    if (Input::IsPressPad(0, XINPUT_GAMEPAD_LEFT_SHOULDER)) {
        pawInputValue = -1.0f;
    } else if (Input::IsPressPad(0, XINPUT_GAMEPAD_RIGHT_SHOULDER)) {
        pawInputValue = 1.0f;
    }

    // deltaTime
    float deltaTime = Frame::DeltaTime();

    // ピッチ
    angleInput_.x = -stickL.y * (speedParam_.pitchSpeed * deltaTime);

    // ヨー
    angleInput_.y = pawInputValue * (speedParam_.yawSpeed * deltaTime);

    // ロール
    float rollInput = -stickL.x;

    // 目標ロール角を更新
    targetRoll_ += rollInput * speedParam_.rollSpeed * deltaTime;

    // 制限
    if (fabs(rollInput) < 0.001f) {
        currentMaxRoll_ = 0.0f;
    } else {
        angleInput_.y   = 0.0f;
        currentMaxRoll_ = ToRadian(rollRotateLimit_);
    }
    targetRoll_ = std::clamp(targetRoll_, -currentMaxRoll_, currentMaxRoll_);
}

void Player::RotateUpdate() {
    float deltaTime = Frame::DeltaTime();

    // ---- 通常のピッチ・ヨー回転処理 ----
    Vector3 targetAngularVelocity = angleInput_;
    const float damping           = 0.95f;
    if (angleInput_.Length() < 0.001f) {
        targetAngularVelocity = angularVelocity_ * damping;
    }
    angularVelocity_ = Lerp(angularVelocity_, targetAngularVelocity, 0.7f);

    Vector3 localRight = GetRightVector();
    Vector3 localUp    = GetUpVector();

    Quaternion pitchRotation = Quaternion::MakeRotateAxisAngle(localRight, angularVelocity_.x * deltaTime);
    Quaternion yawRotation   = Quaternion::MakeRotateAxisAngle(localUp, angularVelocity_.y * deltaTime);

    Quaternion deltaRotation = yawRotation * pitchRotation;
    targetRotation_          = deltaRotation * baseTransform_.quaternion_;
    targetRotation_          = targetRotation_.Normalize();

    // 補正処理
    CorrectionHorizon();

    // 適応
    baseTransform_.quaternion_ = Quaternion::Slerp(
        baseTransform_.quaternion_, targetRotation_, rotationSmoothness_);
    baseTransform_.quaternion_ = baseTransform_.quaternion_.Normalize();

    // ---- ロールを補間 ----
    currentRoll_ = Lerp(currentRoll_, targetRoll_, speedParam_.rollSpeed * deltaTime);

    float yawFromRoll = -sin(currentRoll_) * bankRate_ * deltaTime;
    if (fabs(yawFromRoll) > 0.0001f) {
        Quaternion yawFromRollRotation = Quaternion::MakeRotateAxisAngle(Vector3::ToUp(), yawFromRoll);
        baseTransform_.quaternion_     = yawFromRollRotation * baseTransform_.quaternion_;
        baseTransform_.quaternion_     = baseTransform_.quaternion_.Normalize();
    }

    // ---- obj3dのTransformのみロール適用 ----
    Quaternion visualRoll          = Quaternion::MakeRotateAxisAngle(Vector3::ToForward(), currentRoll_);
    obj3d_->transform_.quaternion_ = visualRoll;
    obj3d_->transform_.quaternion_ = obj3d_->transform_.quaternion_.Normalize();
}
void Player::CorrectionHorizon() {
    float deltaTime = Frame::DeltaTime();

    // 補正開始フラグ
    if (!isAutoRecovering_ && GetIsUpsideDown() && angleInput_.Length() < 0.001f) {
        isAutoRecovering_ = true;
    }

    // 補正処理
    if (!isAutoRecovering_) {
        return;
    }

    Vector3 currentEuler = targetRotation_.ToEuler();
    float currentYaw     = currentEuler.y;

    // 水平の状態
    Quaternion horizontalRotation = Quaternion::EulerToQuaternion(
        Vector3(0.0f, currentYaw, 0.0f));

    // 補正中の値
    Quaternion adjustedCurrent = Quaternion::EulerToQuaternion(
        Vector3(currentEuler.x, currentEuler.y, 0.0f));

    // 補正する
    targetRotation_ = Quaternion::Slerp(
        adjustedCurrent, horizontalRotation, 3.5f * deltaTime);

    // --- 補正終了判定 ---
    if (fabs(currentEuler.x) < 0.01f) {
        isAutoRecovering_ = false;
    }
}

void Player::ReboundByBoundary() {

    // from,toを計算
    Vector3 from = baseTransform_.translation_;
    Vector3 to   = baseTransform_.translation_ + velocity_;

    // FilterMove
    if (moveConstraint_) {
        moveConstraint_->FilterMove(from, to);
    }

    // ブロック判定
    if (RectXZWithGatesConstraint* rectConstraint = dynamic_cast<RectXZWithGatesConstraint*>(moveConstraint_.get())) {
        isRebound_ = rectConstraint->WasBlocked();
    }

    // 跳ね返り処理
    if (ImGui::Button("is")) {
        // 衝突面の法線を計算
        Vector3 collisionNormal = CalculateCollisionNormal(from, to);

        // 入射ベクトル（移動方向）
        Vector3 incidentVector = velocity_.Normalize();

        // 反射ベクトルを計算
        float dotProduct         = Vector3::Dot(incidentVector, collisionNormal);
        Vector3 reflectionVector = incidentVector - collisionNormal * (2.0f * dotProduct);

        // 跳ね返り速度を設定
        reboundVelocity_.y = reflectionVector.y * -reboundPower_;

        // 最後の衝突法線を記録
        lastCollisionNormal_ = collisionNormal;

        float reboundVelocityY = reboundVelocity_.y;

        // 自動入力を開始
        if (reboundVelocityY >= 0.1f) {
            isAutoRotateByCollision = true;
            autoRotateDirection_    = 1.0f;
        } else if (reboundVelocityY <= -0.1f) {
            isAutoRotateByCollision = true;
            autoRotateDirection_    = -1.0f;
        } else {
            isAutoRotateByCollision = false;
            autoRotateDirection_    = 0.0f;
        }
    }

    // 跳ね返り速度の減衰処理
    if (reboundVelocity_.Length() > minReboundVelocity_) {
        // 減衰を適用
        reboundVelocity_.y = reboundVelocity_.y * reboundDecay_;
    } else {
        // 十分小さくなったら停止
        reboundVelocity_ = Vector3::ZeroVector();
        // 跳ね返り回転も停止
        isAutoRotateByCollision = false;
    }
}


Vector3 Player::CalculateCollisionNormal(const Vector3& from, const Vector3& to) {
    // プレイヤーの現在の水平方向を取得
    Vector3 playerForward = GetForwardVector();
    Vector3 playerRight   = GetRightVector();

    // 水平面での移動方向
    Vector3 horizontalMovement = to - from;
    horizontalMovement.y       = 0.0f;

    if (horizontalMovement.Length() < 0.001f) {
        // 移動がほぼない場合はプレイヤーの前方を基準とする
        return -playerForward;
    }

    horizontalMovement = horizontalMovement.Normalize();

    // 境界との衝突方向を推定
    RectXZWithGatesConstraint* rectConstraint = dynamic_cast<RectXZWithGatesConstraint*>(moveConstraint_.get());
    if (rectConstraint) {
        // 矩形の中心を計算（
        Vector3 playerPos = baseTransform_.translation_;

        // 最も近い境界面を特定
        Vector3 normal = Vector3::ZeroVector();

        // X方向の境界チェック
        if (std::abs(playerPos.x - (-1500.0f)) < std::abs(playerPos.x - 1500.0f)) {
            // 左の境界に近い
            normal = Vector3(1.0f, 0.0f, 0.0f); // 右向きの法線
        } else {
            // 右の境界に近い
            normal = Vector3(-1.0f, 0.0f, 0.0f); // 左向きの法線
        }

        // Z方向の境界チェック
        float xDistance = std::min(std::abs(playerPos.x - (-1500.0f)), std::abs(playerPos.x - 1500.0f));
        float zDistance = std::min(std::abs(playerPos.z - (-1500.0f)), std::abs(playerPos.z - 1500.0f));

        if (zDistance < xDistance) {
            // Z方向の境界の方が近い
            if (std::abs(playerPos.z - (-1500.0f)) < std::abs(playerPos.z - 1500.0f)) {
                normal = Vector3(0.0f, 0.0f, 1.0f); // 前向きの法線
            } else {
                normal = Vector3(0.0f, 0.0f, -1.0f); // 後ろ向きの法線
            }
        }

        return normal;
    }

    // 移動方向の逆
    return -horizontalMovement;
}

bool Player::GetIsUpsideDown() const {
    // 機体の上方向ベクトルを取得
    Matrix4x4 targetMatrix = MakeRotateMatrixQuaternion(targetRotation_);
    Vector3 targetUpVector = TransformNormal(Vector3::ToUp(), targetMatrix);

    // 機体の上方向とワールドの上方向の内積を計算
    Vector3 worldUp = Vector3::ToUp();
    float upDot     = Vector3::Dot(targetUpVector, worldUp);

    // 機体が逆さまかどうかを判定
    bool isUpsideDown = upDot < reverseDecisionValue_;

    return isUpsideDown;
}

Vector3 Player::GetForwardVector() const {
    Matrix4x4 rotationMatrix = MakeRotateMatrixQuaternion(baseTransform_.quaternion_);
    return TransformNormal(Vector3::ToForward(), rotationMatrix).Normalize();
}

Vector3 Player::GetRightVector() const {
    Matrix4x4 rotationMatrix = MakeRotateMatrixQuaternion(baseTransform_.quaternion_);
    return TransformNormal(Vector3::ToRight(), rotationMatrix).Normalize();
}

Vector3 Player::GetUpVector() const {
    Matrix4x4 rotationMatrix = MakeRotateMatrixQuaternion(baseTransform_.quaternion_);
    return TransformNormal(Vector3::ToUp(), rotationMatrix).Normalize();
}

void Player::ChangeSpeedBehavior(std::unique_ptr<BasePlayerSpeedBehavior> behavior) {
    if (speedBehavior_) {

        behavior->TransferStateFrom(speedBehavior_.get());
    }
    speedBehavior_ = std::move(behavior);
}
void Player::UpdateSpeedBehavior() {

    auto newBehavior = speedBehavior_->CheckForBehaviorChange();

    if (newBehavior) {

        ChangeSpeedBehavior(std::move(newBehavior));
    }
}

void Player::SpeedInit() {
    speedParam_.currentForwardSpeed = speedParam_.startForwardSpeed;
}

void Player::SpeedUpdate() {
    speedParam_.currentForwardSpeed = speedBehavior_->GetCurrentSpeed();
}

float Player::GetRollDegree() const {
    Vector3 currentEuler = obj3d_->transform_.quaternion_.ToEuler();
    return ToDegree(currentEuler.z);
}

void Player::ReticleDraw() {
    reticle_->Draw();
}

void Player::BindParams() {
    globalParameter_->Bind(groupName_, "hp", &hp_);
    globalParameter_->Bind(groupName_, "forwardSpeed", &speedParam_.startForwardSpeed);
    globalParameter_->Bind(groupName_, "pitchSpeed", &speedParam_.pitchSpeed);
    globalParameter_->Bind(groupName_, "yawSpeed", &speedParam_.yawSpeed);
    globalParameter_->Bind(groupName_, "rollSpeed", &speedParam_.rollSpeed);
    globalParameter_->Bind(groupName_, "minForwardSpeed", &speedParam_.minForwardSpeed);
    globalParameter_->Bind(groupName_, "maxForwardSpeed", &speedParam_.maxForwardSpeed);
    globalParameter_->Bind(groupName_, "brakeForwardSpeed", &speedParam_.brakeForwardSpeed);
    globalParameter_->Bind(groupName_, "rotationSmoothness", &rotationSmoothness_);
    globalParameter_->Bind(groupName_, "rollRotateLimit", &rollRotateLimit_);
    globalParameter_->Bind(groupName_, "pitchBackTime", &pitchBackTime_);
    globalParameter_->Bind(groupName_, "rollBackTime", &rollBackTime_);
    globalParameter_->Bind(groupName_, "pitchReturnThreshold", &pitchReturnThreshold_);
    globalParameter_->Bind(groupName_, "reverseDecisionValue", &reverseDecisionValue_);
    globalParameter_->Bind(groupName_, "bankRate", &bankRate_);
    globalParameter_->Bind(groupName_, "reboundPower", &reboundPower_);
    globalParameter_->Bind(groupName_, "reboundDecay", &reboundDecay_);
    globalParameter_->Bind(groupName_, "minReboundVelocity", &minReboundVelocity_);
}

void Player::AdjustParam() {
#ifdef _DEBUG
    if (ImGui::CollapsingHeader(groupName_.c_str())) {
        ImGui::PushID(groupName_.c_str());

        ImGui::DragInt("Hp", &hp_);

        // EditParameter
        ImGui::Separator();
        ImGui::Text("Fighter Controls");
        ImGui::SeparatorText("Speed");
        ImGui::DragFloat("StartForward Speed", &speedParam_.startForwardSpeed, 0.01f);
        ImGui::DragFloat("minForward Speed", &speedParam_.minForwardSpeed, 0.01f);
        ImGui::DragFloat("maxForward Speed", &speedParam_.maxForwardSpeed, 0.01f);
        ImGui::DragFloat("brakeForward Speed", &speedParam_.brakeForwardSpeed, 0.01f);
        ImGui::DragFloat("Pitch Speed", &speedParam_.pitchSpeed, 0.01f);
        ImGui::DragFloat("Yaw Speed", &speedParam_.yawSpeed, 0.01f);
        ImGui::DragFloat("Roll Speed", &speedParam_.rollSpeed, 0.01f);

        ImGui::SeparatorText("rebound");
        ImGui::DragFloat("rebound Power", &reboundPower_, 0.01f);
        ImGui::DragFloat("rebound Decay", &reboundDecay_, 0.01f, 0.0f, 1.0f);
        ImGui::DragFloat("min Rebound Velocity", &minReboundVelocity_, 0.01f, 0.0f, 10.0f);

        ImGui::SeparatorText("etc");
        ImGui::DragFloat("rotationSmoothness", &rotationSmoothness_, 0.01f, 0.0f, 1.0f);
        ImGui::DragFloat("rollRotateLimit", &rollRotateLimit_, 0.01f);
        ImGui::DragFloat("pitchBackTime", &pitchBackTime_, 0.01f, 0.0f, 5.0f);
        ImGui::DragFloat("rollBackTime", &rollBackTime_, 0.01f, 0.0f, 5.0f);
        ImGui::DragFloat("pitchReturnThreshold", &pitchReturnThreshold_, 1.0f, 0.0f, 90.0f);
        ImGui::DragFloat("reverseDecisionValue", &reverseDecisionValue_, 0.01f, -1.0f, 0.0f);
        ImGui::DragFloat("bankRate", &bankRate_, 0.01f);

        // デバッグ
        ImGui::Separator();
        ImGui::Text("Debug Info");
        ImGui::Text("Position: (%.2f, %.2f, %.2f)",
            baseTransform_.translation_.x,
            baseTransform_.translation_.y,
            baseTransform_.translation_.z);
        ImGui::Text("Velocity: (%.2f, %.2f, %.2f)", velocity_.x, velocity_.y, velocity_.z);
        ImGui::Text("Rebound Velocity: (%.2f, %.2f, %.2f)",
            reboundVelocity_.x, reboundVelocity_.y, reboundVelocity_.z);
        ImGui::Text("Collision Normal: (%.2f, %.2f, %.2f)",
            lastCollisionNormal_.x, lastCollisionNormal_.y, lastCollisionNormal_.z);

        // 機体姿勢のデバッグ
        ImGui::Separator();
        ImGui::Text("Aircraft Attitude");
        Vector3 euler = baseTransform_.quaternion_.ToEuler();
        ImGui::Text("Euler (deg): P=%.1f, Y=%.1f, R=%.1f",
            ToDegree(euler.x), ToDegree(euler.y), ToDegree(euler.z));

        if (GetIsUpsideDown()) {
            ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "STATUS: UPSIDE DOWN!");
        } else {
            ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "STATUS: Normal");
        }

        if (isRebound_) {
            ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "REBOUNDING!");
        }

        ImGui::Text("currentSpeed:%.3f", speedParam_.currentForwardSpeed);

        // セーブ・ロード
        ImGui::Separator();
        globalParameter_->ParamSaveForImGui(groupName_);
        globalParameter_->ParamLoadForImGui(groupName_);

        ImGui::PopID();
    }
    // 弾
    if (bulletShooter_) {
        bulletShooter_->AdjustParam();
    }
    // レティクル
    if (reticle_) {
        reticle_->AdjustParam();
    }

#endif // _DEBUG
}