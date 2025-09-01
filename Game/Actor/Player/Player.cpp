#include "Player.h"
#include "input/Input.h"
#include "MathFunction.h"
#include"Frame/Frame.h"
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
    obj3d_->transform_.parent_ = &baseTransform_;

    // 戦闘機用パラメータ初期化
    forwardSpeed_ = 50.0f; // 前進速度
    pitchSpeed_   = 2.0f; // ピッチ速度
    yawSpeed_     = 1.5f; // ヨー速度
    rollSpeed_    = 3.0f; // ロール速度

    velocity_        = Vector3::ZeroVector();
    angularVelocity_ = Vector3::ZeroVector();
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
    Vector2 stickL = {0.0f, 0.0f};
    Vector2 stickR = {0.0f, 0.0f};

    // ゲームパッドの入力
    stickL = Input::GetPadStick(0, 0); 
    stickR = Input::GetPadStick(0, 1); 

    // キーボード入力
    if (stickL.Length() < 0.1f && stickR.Length() < 0.1f) {
        
        if (input->PushKey(DIK_W))
            stickL.y = 1.0f;
        if (input->PushKey(DIK_S))
            stickL.y = -1.0f; 
        if (input->PushKey(DIK_A))
            stickL.x = -1.0f; 
        if (input->PushKey(DIK_D))
            stickL.x = 1.0f; 
     
        if (input->PushKey(DIK_LEFT))
            stickR.x = -1.0f; 
        if (input->PushKey(DIK_RIGHT))
            stickR.x = 1.0f;
    }

    // コントローラ処理
    if (stickL.Length() > 0.1f) {
        stickL = stickL.Normalize() * std::min(stickL.Length(), 1.0f);
    } else {
        stickL = {0.0f, 0.0f};
    }

    if (stickR.Length() > 0.1f) {
        stickR = stickR.Normalize() * std::min(stickR.Length(), 1.0f);
    } else {
        stickR = {0.0f, 0.0f};
    }

    // deltaTime
    float deltaTime = Frame::DeltaTime();

    // ピッチ
    targetAngularVelocity_.x = -stickL.y * (pitchSpeed_ * deltaTime);

    // ヨー
    targetAngularVelocity_.y = stickR.x * (yawSpeed_ * deltaTime);

    // ロール
    targetAngularVelocity_.z = -stickL.x * (rollSpeed_ * deltaTime);
}

void Player::UpdatePhysics() {
    float deltaTime = Frame::DeltaTime();

    // 角速度の補間
    angularVelocity_ = Lerp(angularVelocity_, targetAngularVelocity_, 0.1f);

    // 回転を適用
    baseTransform_.rotation_ += angularVelocity_;

    // ロール角度を制限
    if (baseTransform_.rotation_.z > std::numbers::pi_v<float>) {
        baseTransform_.rotation_.z -= 2.0f * std::numbers::pi_v<float>;
    }
    if (baseTransform_.rotation_.z < -std::numbers::pi_v<float>) {
        baseTransform_.rotation_.z += 2.0f * std::numbers::pi_v<float>;
    }

    // 前方ベクトルを計算
    Matrix4x4 rotationMatrix = MakeRotateMatrix(baseTransform_.rotation_);
    Vector3 forward          = TransformNormal(Vector3::ToForward(), rotationMatrix);

    // 進行方向に前進
    velocity_ = forward * forwardSpeed_ * deltaTime;

    // 位置を更新
    baseTransform_.translation_ += velocity_;
}

void Player::Move() {
   
}

Vector3 Player::GetForwardVector() const {
    Matrix4x4 rotationMatrix = MakeRotateMatrix(baseTransform_.rotation_);
    return TransformNormal(Vector3::ToForward(), rotationMatrix);
}

Vector3 Player::GetRightVector() const {
    Matrix4x4 rotationMatrix = MakeRotateMatrix(baseTransform_.rotation_);
    return TransformNormal(Vector3::ToRight(), rotationMatrix);
}

Vector3 Player::GetUpVector() const {
    Matrix4x4 rotationMatrix = MakeRotateMatrix(baseTransform_.rotation_);
    return TransformNormal(Vector3::ToUp(), rotationMatrix);
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
        ImGui::DragFloat("Forward Speed", &forwardSpeed_, 1.0f, 10.0f, 200.0f);
        ImGui::DragFloat("Pitch Speed", &pitchSpeed_, 0.1f, 0.5f, 5.0f);
        ImGui::DragFloat("Yaw Speed", &yawSpeed_, 0.1f, 0.5f, 5.0f);
        ImGui::DragFloat("Roll Speed", &rollSpeed_, 0.1f, 0.5f, 5.0f);

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