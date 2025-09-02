#include "Player.h"
#include "Frame/Frame.h"
#include "input/Input.h"
#include "MathFunction.h"
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

    targetRotation_     = Quaternion::Identity();
    rotationSmoothness_ = 0.1f;
}

void Player::Update() {
    // 入力処理
    HandleInput();

    // 物理計算
    UpdatePhysics();

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
}
void Player::UpdatePhysics() {
    float deltaTime = Frame::DeltaTime();

    // 角速度の補間
    angularVelocity_ = Lerp(angularVelocity_, angleInput_, 0.3f);

    // 現在の回転をオイラー角に変換
    Vector3 currentEuler = baseTransform_.quaternion_.ToEuler();

    // 現在のロール角（Z軸）
    float currentRoll = currentEuler.z;

    // 次フレームでの予測ロール角
    float predictedRoll = currentRoll + angularVelocity_.z * deltaTime;

    // 制限角度
    const float maxRoll = std::numbers::pi_v<float> / 2.0f;

    // 上限チェック
    if (predictedRoll > maxRoll) {
        predictedRoll      = maxRoll;
        angularVelocity_.z = 0.0f;
    }
    // 下限チェック
    else if (predictedRoll < -maxRoll) {
        predictedRoll      = -maxRoll;
        angularVelocity_.z = 0.0f;
    }

    // 現在の姿勢からローカル軸を取得
    Vector3 localRight   = GetRightVector();
    Vector3 localUp      = GetUpVector();
    Vector3 localForward = GetForwardVector();

    // 各軸周りの回転をQuaternionで作成
    Quaternion pitchRotation = Quaternion::MakeRotateAxisAngle(localRight, angularVelocity_.x * deltaTime);
    Quaternion yawRotation   = Quaternion::MakeRotateAxisAngle(localUp, angularVelocity_.y * deltaTime);
    Quaternion rollRotation  = Quaternion::MakeRotateAxisAngle(localForward, angularVelocity_.z * deltaTime);

    // 合成回転を適用
    Quaternion deltaRotation = pitchRotation * yawRotation * rollRotation;

    targetRotation_ = deltaRotation * baseTransform_.quaternion_;
    targetRotation_ = targetRotation_.Normalize();

    if (fabs(angleInput_.z) < 0.001f) {
        Vector3 euler      = targetRotation_.ToEuler();
        float currentYaw   = euler.y;
        float currentPitch = euler.x;

        // ロールだけゼロにした姿勢を作成
        Quaternion noRollRotation = Quaternion::EulerToQuaternion(
            Vector3(currentPitch, currentYaw, 0.0f));

        // スムーズに補間
        targetRotation_ = Quaternion::Slerp(
            targetRotation_,
            noRollRotation,
            rollBackTime_ * deltaTime);
    }

    // 回転補間
    baseTransform_.quaternion_ = Quaternion::Slerp(
        baseTransform_.quaternion_,
        targetRotation_,
        rotationSmoothness_);
    baseTransform_.quaternion_ = baseTransform_.quaternion_.Normalize();

    // 前方ベクトルをQuaternionから計算
    Vector3 forward = GetForwardVector();

    // 常に前進
    velocity_ = forward * forwardSpeed_ * deltaTime;

    // 位置を更新
    baseTransform_.translation_ += velocity_;
}

void Player::Move() {
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
///=========================================================
/// バインド
///==========================================================
void Player::BindParams() {
    globalParameter_->Bind(groupName_, "hp", &hp_);
    globalParameter_->Bind(groupName_, "speed", &speed_);
    globalParameter_->Bind(groupName_, "forwardSpeed", &forwardSpeed_);
    globalParameter_->Bind(groupName_, "pitchSpeed", &pitchSpeed_);
    globalParameter_->Bind(groupName_, "yawSpeed", &yawSpeed_);
    globalParameter_->Bind(groupName_, "rollSpeed", &rollSpeed_);
    globalParameter_->Bind(groupName_, "rollBackTime", &rollBackTime_);
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
        ImGui::DragFloat("RollBackTime", &rollBackTime_, 0.01f);

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

        // セーブ・ロード
        globalParameter_->ParamSaveForImGui(groupName_);
        globalParameter_->ParamLoadForImGui(groupName_);

        ImGui::PopID();
    }
#endif // _DEBUG
}

///==========================================================
/// 移動
///==========================================================
void Player::DirectionToCamera() {
}