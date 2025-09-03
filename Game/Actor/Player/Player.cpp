#include "Player.h"
#include "Frame/Frame.h"
#include "input/Input.h"
#include "MathFunction.h"
// behavior
#include "Behavior/PlayerBoost.h"
#include "Behavior/PlayerSpeedDown.h"
#include <imgui.h>
#include <numbers>

void Player::Init() {

    // グローバルパラメータ
    globalParameter_ = GlobalParameter::GetInstance();
    globalParameter_->CreateGroup(groupName_, false);
    BindParams();
    globalParameter_->SyncParamForGroup(groupName_);

    // モデル作成
    obj3d_.reset(Object3d::CreateModel("Player.obj"));

    // transform初期化
    baseTransform_.Init();
    baseTransform_.rotateOder_ = RotateOder::Quaternion;
    baseTransform_.quaternion_ = Quaternion::Identity();
    obj3d_->transform_.parent_ = &baseTransform_;

    velocity_        = Vector3::ZeroVector();
    angularVelocity_ = Vector3::ZeroVector();
    targetRotation_  = Quaternion::Identity();

    bulletShooter_ = std::make_unique<PlayerBulletShooter>();
    bulletShooter_->Init();

    ChangeSpeedBehavior(std::make_unique<PlayerSpeedDown>(this));
}

void Player::Update() {
    // 入力処理
    HandleInput();

    // 物理計算
    RotateUpdate();

    // 弾丸システム更新
    if (bulletShooter_) {
        bulletShooter_->Update(this);
    }

    // トランスフォーム更新
    BaseObject::Update();
}

void Player::HandleInput() {
    Input* input = Input::GetInstance();

    // 入力値をリセット
    Vector2 stickL = Vector2::ZeroVector();
    Vector2 stickR = Vector2::ZeroVector();

    // ゲームパッドの入力
    stickL = Input::GetPadStick(0, 0);
    stickR = Input::GetPadStick(0, 1);

    // キーボード入力
    if (stickL.Length() < 0.1f && stickR.Length() < 0.1f) {

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

    // コントローラ処理
    if (stickL.Length() > 0.1f) {
        stickL = stickL.Normalize() * std::min(stickL.Length(), 1.0f);
    } else {
        stickL = {0.0f, 0.0f};
    }

    if (Input::IsPressPad(0, XINPUT_GAMEPAD_LEFT_SHOULDER)) {
        stickR.x = -1.0f;
    } else if (Input::IsPressPad(0, XINPUT_GAMEPAD_RIGHT_SHOULDER)) {
        stickR.x = 1.0f;
    }

    // deltaTime
    float deltaTime = Frame::DeltaTime();

    // ピッチ
    angleInput_.x = -stickL.y * (pitchSpeed_ * deltaTime);

    // ヨー
    angleInput_.y = stickR.x * (yawSpeed_ * deltaTime);

    // ロール
    angleInput_.z = -stickL.x * (rollSpeed_ * deltaTime);

    // 現在のオイラー角を取得
    Vector3 currentEuler = baseTransform_.quaternion_.ToEuler();
    float currentRoll    = currentEuler.z;

    // 最大ロール角
    const float maxRoll = ToRadian(rollRotateLimit_);

    // 入力値に基づくロール回転
    float rollInput = -stickL.x * (rollSpeed_ * deltaTime);

    // ロール制限処理
    if ((currentRoll > maxRoll && rollInput > 0.0f) || (currentRoll < -maxRoll && rollInput < 0.0f)) {

        rollInput = maxRoll;
    }

    // 最終的なロール角速度入力
    angleInput_.z = rollInput;
}

void Player::RotateUpdate() {
    float deltaTime = Frame::DeltaTime();

    // 角速度の補間
    Vector3 targetAngularVelocity = angleInput_;

    // 入力がない場合は減衰させる
    const float damping = 0.95f;
    if (angleInput_.Length() < 0.001f) {
        targetAngularVelocity = angularVelocity_ * damping;
    }

    // 角速度変化
    angularVelocity_ = Lerp(angularVelocity_, targetAngularVelocity, 0.7f);

    // 現在の姿勢からローカル軸を取得
    Vector3 localRight   = GetRightVector();
    Vector3 localUp      = GetUpVector();
    Vector3 localForward = GetForwardVector();

    // 各軸周りの回転をQuaternionで作成
    Quaternion pitchRotation = Quaternion::MakeRotateAxisAngle(localRight, angularVelocity_.x * deltaTime);
    Quaternion yawRotation   = Quaternion::MakeRotateAxisAngle(localUp, angularVelocity_.y * deltaTime);
    Quaternion rollRotation  = Quaternion::MakeRotateAxisAngle(localForward, angularVelocity_.z * deltaTime);

    // ヨーピッチロール
    Quaternion deltaRotation = yawRotation * pitchRotation * rollRotation;

    // 回転を適用
    targetRotation_ = deltaRotation * baseTransform_.quaternion_;
    targetRotation_ = targetRotation_.Normalize();

    // 機体の上方向ベクトルを取得
    Matrix4x4 targetMatrix = MakeRotateMatrixQuaternion(targetRotation_);
    Vector3 targetUpVector = TransformNormal(Vector3::ToUp(), targetMatrix);

    // 機体の上方向とワールドの上方向の内積を計算
    Vector3 worldUp = Vector3::ToUp();
    float upDot     = Vector3::Dot(targetUpVector, worldUp);

    // 機体が逆さまかどうかを判定
    const float upsideDownThreshold = -0.3f;
    bool isUpsideDown               = upDot < upsideDownThreshold;

    // 補正開始フラグ
    if (!isAutoRecovering_ && isUpsideDown && angleInput_.Length() < 0.001f) {
        isAutoRecovering_ = true;
    }

    // 補正処理
    if (isAutoRecovering_) {
        Vector3 currentEuler = targetRotation_.ToEuler();
        float currentYaw     = currentEuler.y;

        // 目標は水平 
        float targetPitch = 0.0f;
        float targetRoll  = 0.0f;

        Quaternion horizontalRotation = Quaternion::EulerToQuaternion(
            Vector3(targetPitch, currentYaw, targetRoll));

        //ロール補正
        float roll = currentEuler.z;
        if (roll < 0.0f) {
            roll += 2.0f * std::numbers::pi_v<float>;
            Quaternion adjustedCurrent = Quaternion::EulerToQuaternion(
                Vector3(currentEuler.x, currentEuler.y, roll));
            targetRotation_ = Quaternion::Slerp(
                adjustedCurrent, horizontalRotation, 3.5f * deltaTime);
        } else {
            targetRotation_ = Quaternion::Slerp(
                targetRotation_, horizontalRotation, 3.5f * deltaTime);
        }

        // --- 補正終了判定 ---
        if (fabs(currentEuler.x) < 0.01f && fabs(currentEuler.z) < 0.01f) {
            isAutoRecovering_ = false;
        }
    }


    // 通常時の自動復帰処理
    else if (!isUpsideDown && angleInput_.Length() < 0.001f) {
        Vector3 euler      = targetRotation_.ToEuler();
        float currentYaw   = euler.y;
        float currentPitch = euler.x;
        float currentRoll  = euler.z;

        // ロール入力なしの場合のみロールを0に戻す
        if (fabs(angleInput_.z) < 0.001f) {
            currentRoll = Lerp(currentRoll, 0.0f, rollBackTime_ * deltaTime);
        }

        // 補正された回転を適用
        targetRotation_ = Quaternion::EulerToQuaternion(
            Vector3(currentPitch, currentYaw, currentRoll));
    }

    // 最終的な回転補間
    baseTransform_.quaternion_ = Quaternion::Slerp(
        baseTransform_.quaternion_,
        targetRotation_,
        rotationSmoothness_);
    baseTransform_.quaternion_ = baseTransform_.quaternion_.Normalize();

    // 現在のオイラー角を取得
    Vector3 currentEuler = baseTransform_.quaternion_.ToEuler();
    float currentRoll    = currentEuler.z;

    // 前方・右・上方向ベクトルを取得
    Vector3 forward = GetForwardVector();
    Vector3 right   = GetRightVector();
    Vector3 up      = GetUpVector();

    // ロールに基づくヨー角速度を計算
    float rollToYawRate = 0.7f;
    float yawFromRoll   = -sin(currentRoll) * rollToYawRate * deltaTime;

    // ヨー回転をQuaternionで作成
    if (fabs(yawFromRoll) > 0.0001f) {
        Quaternion yawFromRollRotation = Quaternion::MakeRotateAxisAngle(
            Vector3::ToUp(), // ワールド上方向
            yawFromRoll);

        // 回転を適用
        baseTransform_.quaternion_ = yawFromRollRotation * baseTransform_.quaternion_;
        baseTransform_.quaternion_ = baseTransform_.quaternion_.Normalize();
    }

    // 速度計算
    Vector3 forwardVelocity = forward * forwardSpeed_ * deltaTime;

    // 合成速度
    velocity_ = forwardVelocity;

    // 位置を更新
    baseTransform_.translation_ += velocity_;
}

void Player::SpeedChange() {
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

///========================================================================
/// バインド
///========================================================================
void Player::BindParams() {
    globalParameter_->Bind(groupName_, "hp", &hp_);
    globalParameter_->Bind(groupName_, "speed", &speed_);
    globalParameter_->Bind(groupName_, "forwardSpeed", &forwardSpeed_);
    globalParameter_->Bind(groupName_, "pitchSpeed", &pitchSpeed_);
    globalParameter_->Bind(groupName_, "yawSpeed", &yawSpeed_);
    globalParameter_->Bind(groupName_, "rollSpeed", &rollSpeed_);
    globalParameter_->Bind(groupName_, "rotationSmoothness", &rotationSmoothness_);
    globalParameter_->Bind(groupName_, "rollRotateLimit", &rollRotateLimit_);
    globalParameter_->Bind(groupName_, "pitchBackTime", &pitchBackTime_);
    globalParameter_->Bind(groupName_, "rollBackTime", &rollBackTime_);
    globalParameter_->Bind(groupName_, "pitchReturnThreshold", &pitchReturnThreshold_);
    globalParameter_->Bind(groupName_, "sideFactor", &sideFactor_);
    globalParameter_->Bind(groupName_, "downFactor", &downFactor_);
}

///=========================================================
/// パラメータ調整
///==========================================================
void Player::AdjustParam() {
#ifdef _DEBUG
    if (ImGui::CollapsingHeader(groupName_.c_str())) {
        ImGui::PushID(groupName_.c_str());

        ImGui::DragInt("Hp", &hp_);
        ImGui::DragFloat("speed", &speed_, 0.01f);

        // EditParameter
        ImGui::Separator();
        ImGui::Text("Fighter Controls");
        ImGui::DragFloat("Forward Speed", &forwardSpeed_, 0.01f);
        ImGui::DragFloat("Pitch Speed", &pitchSpeed_, 0.01f);
        ImGui::DragFloat("Yaw Speed", &yawSpeed_, 0.01f);
        ImGui::DragFloat("Roll Speed", &rollSpeed_, 0.01f);
        ImGui::DragFloat("rotationSmoothness", &rotationSmoothness_, 0.01f, 0.0f, 1.0f);
        ImGui::DragFloat("rollRotateLimit", &rollRotateLimit_, 0.01f);
        ImGui::DragFloat("pitchBackTime", &pitchBackTime_, 0.01f, 0.0f, 5.0f);
        ImGui::DragFloat("rollBackTime", &rollBackTime_, 0.01f, 0.0f, 5.0f);
        ImGui::DragFloat("pitchReturnThreshold", &pitchReturnThreshold_, 1.0f, 0.0f, 90.0f);
        ImGui::DragFloat("sideFactor", &sideFactor_, 0.01f);
        ImGui::DragFloat("downFactor", &downFactor_, 0.01f);

        // デバッグ
        ImGui::Separator();
        ImGui::Text("Debug Info");
        ImGui::Text("Position: (%.2f, %.2f, %.2f)",
            baseTransform_.translation_.x,
            baseTransform_.translation_.y,
            baseTransform_.translation_.z);
        ImGui::Text("Rotation: (%.2f, %.2f, %.2f)",
            baseTransform_.rotation_.x * 180.0f / std::numbers::pi_v<float>,
            baseTransform_.rotation_.y * 180.0f / std::numbers::pi_v<float>,
            baseTransform_.rotation_.z * 180.0f / std::numbers::pi_v<float>);
        ImGui::Text("Velocity: (%.2f, %.2f, %.2f)", velocity_.x, velocity_.y, velocity_.z);

        // 機体姿勢のデバッグ
        ImGui::Separator();
        ImGui::Text("Aircraft Attitude");
        Vector3 euler = baseTransform_.quaternion_.ToEuler();
        ImGui::Text("Euler (deg): P=%.1f, Y=%.1f, R=%.1f",
            ToDegree(euler.x),
            ToDegree(euler.y),
            ToDegree(euler.z));

        // 機体の上方向ベクトル
        Matrix4x4 rotMatrix = MakeRotateMatrixQuaternion(baseTransform_.quaternion_);
        Vector3 upVector    = TransformNormal(Vector3::ToUp(), rotMatrix);
        Vector3 worldUp     = Vector3(0.0f, 1.0f, 0.0f);
        float upDot         = Vector3::Dot(upVector, worldUp);

        ImGui::Text("Up Vector: (%.2f, %.2f, %.2f)", upVector.x, upVector.y, upVector.z);
        ImGui::Text("Up Dot Product: %.3f", upDot);

        // 逆さま判定の表示
        bool isUpsideDown = upDot < -0.3f;
        if (isUpsideDown) {
            ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "STATUS: UPSIDE DOWN!");
        } else {
            ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "STATUS: Normal");
        }

        // セーブ・ロード
        ImGui::Separator();
        globalParameter_->ParamSaveForImGui(groupName_);
        globalParameter_->ParamLoadForImGui(groupName_);

        ImGui::PopID();
    }
    if (bulletShooter_) {
        bulletShooter_->AdjustParam();
    }
#endif // _DEBUG
}

///==========================================================
/// 移動
///==========================================================
void Player::DirectionToCamera() {
}

void Player::ChangeSpeedBehavior(std::unique_ptr<BasePlayerSpeedBehavior> behavior) {
    speedBehavior_ = std::move(behavior);
}

void Player::UpdateSpeedBehavior() {

    // LBボタンの状態を取得
    isLBPressed_ = Input::IsPressPad(0, XINPUT_GAMEPAD_RIGHT_SHOULDER);

    // ボタンが押された瞬間の処理
    if (isLBPressed_ && !wasLBPressed_) {
        // BoostBehaviorに切り替え
        ChangeSpeedBehavior(std::make_unique<PlayerBoost>(this));
    }
    // ボタンが離された瞬間の処理
    else if (!isLBPressed_ && wasLBPressed_) {
        // SpeedDownBehaviorに切り替え
        ChangeSpeedBehavior(std::make_unique<PlayerSpeedDown>(this));
    }

    // 前フレームのボタン状態を保存
    wasLBPressed_ = isLBPressed_;
}