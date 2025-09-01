#include "GameCamera.h"
// Function
#include "MathFunction.h"
// math
#include "Matrix4x4.h"
// input
#include "input/Input.h"

/// std
#include <imgui.h>
#include <numbers>

void GameCamera::Init() {
    viewProjection_.Init();

    ///* グローバルパラメータ
    globalParameter_ = GlobalParameter::GetInstance();
    globalParameter_->CreateGroup(groupName_, false);
    BindParams();
    globalParameter_->SyncParamForGroup(groupName_);

    rotate_ = baseRotateOffsetX_;
    offset_ = offset_;

    rendition_ = std::make_unique<CameraRendition>();
    rendition_->Init();
    rendition_->SetGameCamera(this);
    rendition_->SetViewProjection(&viewProjection_);

    // 背後追従カメラのデフォルト設定
    followDistance_     = 10.0f;
    followHeight_       = 2.0f;
    followSmoothness_   = 0.1f;
    rotationSmoothness_ = 0.15f;
    isFollowMode_       = true;
}

void GameCamera::Update() {

    rendition_->Update();
    shakeOffsetPos_ = rendition_->GetShakeOffset();

    // カメラの基本移動処理
    if (isFollowMode_ && target_) {
        FollowUpdate();
    } else {
        MoveUpdate();
    }

    // ビュー行列の更新
    viewProjection_.UpdateMatrix();
}

void GameCamera::FollowUpdate() {
    if (!target_)
        return;

    // プレイヤーの位置とQuaternionを取得
    Vector3 playerPos = target_->GetWorldPos();

    // プレイヤーのQuaternionから正確な方向ベクトルを取得
    Vector3 playerForward, playerUp;

    // WorldTransformのQuaternionから方向ベクトルを計算
    if (target_->rotateOder_ == RotateOder::Quaternion) {
        Matrix4x4 rotationMatrix = MakeRotateMatrixQuaternion(target_->quaternion_);
        playerForward            = TransformNormal(Vector3::ToForward(), rotationMatrix).Normalize();
        playerUp                 = TransformNormal(Vector3::ToUp(), rotationMatrix).Normalize();
    } else {
        // 従来のオイラー角ベース
        Matrix4x4 rotationMatrix = MakeRotateMatrix(target_->rotation_);
        playerForward            = TransformNormal(Vector3::ToForward(), rotationMatrix).Normalize();
        playerUp                 = TransformNormal(Vector3::ToUp(), rotationMatrix).Normalize();
    }

    // カメラの目標位置を計算（プレイヤーの後方 + 高さオフセット）
    Vector3 targetCameraPos = playerPos - (playerForward * followDistance_) + (playerUp * followHeight_);

    // シェイクオフセットを適用
    targetCameraPos += shakeOffsetPos_;

    // カメラ位置を補間
    viewProjection_.translation_ = Lerp(viewProjection_.translation_, targetCameraPos, followSmoothness_);

    // カメラの向きを計算（プレイヤーを見る方向）
    Vector3 cameraToPlayer = (playerPos - viewProjection_.translation_).Normalize();

    // カメラの目標回転を計算
    Vector3 targetRotation = CalculateLookAtRotation(cameraToPlayer, playerUp);

    // 回転を補間
    viewProjection_.rotation_.x = LerpShortAngle(viewProjection_.rotation_.x, targetRotation.x, rotationSmoothness_);
    viewProjection_.rotation_.y = LerpShortAngle(viewProjection_.rotation_.y, targetRotation.y, rotationSmoothness_);
    viewProjection_.rotation_.z = LerpShortAngle(viewProjection_.rotation_.z, targetRotation.z, rotationSmoothness_);
}

Vector3 GameCamera::CalculateLookAtRotation(const Vector3& direction, const Vector3& up) {
    // 正規化された方向ベクトル
    Vector3 forward = direction.Normalize();

    // 右ベクトルを計算
    Vector3 right = Vector3::Cross(up, forward).Normalize();

    // 実際の上ベクトルを再計算
    Vector3 actualUp = Vector3::Cross(forward, right).Normalize();

    // 回転角度を計算
    Vector3 rotation;

    // ピッチ（X軸回転）
    rotation.x = std::asin(-forward.y);

    // ヨー（Y軸回転）
    rotation.y = std::atan2(forward.x, forward.z);

    // ロール（Z軸回転）- 通常は0に近い値
    rotation.z = std::atan2(right.y, actualUp.y);

    return rotation;
}

void GameCamera::MoveUpdate() {
    ///============================================================
    // 入力処理
    ///============================================================
    Input* input            = Input::GetInstance();
    const float rotateSpeed = 0.08f;
    Vector2 stickInput      = {0.0f, 0.0f};

    // ================================= keyBord ================================= //
    if (input->PushKey(DIK_RIGHT) || input->PushKey(DIK_LEFT)) {

        if (input->PushKey(DIK_RIGHT)) { // 右回転
            stickInput.x = 1.0f;

        } else if (input->PushKey(DIK_LEFT)) { // 左回転
            stickInput.x = -1.0f;
        }

    } else {
        // ================================= GamePad ================================= //
        stickInput = Input::GetPadStick(0, 1); // 右スティックの入力を取得
    }

    ///============================================================
    // 入力に対する回転処理
    ///============================================================
    if (stickInput.Length() > 0.1f) {
        stickInput = stickInput.Normalize();
        destinationAngleY_ += stickInput.x * rotateSpeed;
    }

    // reset
    if (input->TrrigerKey(DIK_R) || Input::IsTriggerPad(0, XINPUT_GAMEPAD_RIGHT_THUMB)) {
        Reset();
    }

    // フォローモード切り替え
    if (input->TrrigerKey(DIK_F) || Input::IsTriggerPad(0, XINPUT_GAMEPAD_Y)) {
        ToggleFollowMode();
    }

    // Y軸の回転補間処理
    viewProjection_.rotation_.y = LerpShortAngle(viewProjection_.rotation_.y, destinationAngleY_, 0.3f);

    
}

void GameCamera::RotateAdapt() {
    // Y軸の回転補間処理
    viewProjection_.rotation_.y = LerpShortAngle(viewProjection_.rotation_.y, destinationAngleY_, 0.3f);

    // 見下ろし角度の固定
    const float fixedPitchAngle = rotate_ * std::numbers::pi_v<float> / 180.0f;
    viewProjection_.rotation_.x = fixedPitchAngle;
}

void GameCamera::TranslateAdapt() {
    if (!target_)
        return;
    interTarget_                 = Lerp(interTarget_, target_->translation_, 1.0f);
    Vector3 offset               = OffsetCalc(offset_);
    viewProjection_.translation_ = interTarget_ + offset;
}

void GameCamera::Reset() {
    // 追従対象がいれば
    if (target_) {
        // 追従座標・角度の初期化
        interTarget_ = target_->translation_;
        if (!isFollowMode_) {
            viewProjection_.rotation_.y = LerpShortAngle(viewProjection_.rotation_.y, target_->rotation_.y, 0.3f);
        }
    }
    destinationAngleY_ = viewProjection_.rotation_.y;

    if (!isFollowMode_) {
        // 追従対象からのオフセット
        Vector3 offset               = OffsetCalc(offset_);
        viewProjection_.translation_ = interTarget_ + offset;
    }
}

void GameCamera::ToggleFollowMode() {
    isFollowMode_ = !isFollowMode_;
    if (isFollowMode_ && target_) {
        // フォローモードに切り替えたときの初期化処理
        Reset();
    }
}

Vector3 GameCamera::OffsetCalc(const Vector3& offset) const {
    // カメラの角度から回転行列を計算する
    Matrix4x4 rotateMatrix = MakeRotateYMatrix(viewProjection_.rotation_.y);
    Vector3 resultOffset   = TransformNormal(offset + shakeOffsetPos_, rotateMatrix);
    return resultOffset;
}

void GameCamera::BindParams() {
    globalParameter_->Bind(groupName_, "RotateOffsetX", &baseRotateOffsetX_);
    globalParameter_->Bind(groupName_, "offset", &offset_);
    globalParameter_->Bind(groupName_, "followDistance", &followDistance_);
    globalParameter_->Bind(groupName_, "followHeight", &followHeight_);
    globalParameter_->Bind(groupName_, "followSmoothness", &followSmoothness_);
    globalParameter_->Bind(groupName_, "rotationSmoothness", &rotationSmoothness_);
}

void GameCamera::AdjustParam() {
#ifdef _DEBUG
    if (ImGui::CollapsingHeader(groupName_.c_str())) {
        ImGui::PushID(groupName_.c_str());

        // フォローモードの切り替え
        ImGui::Checkbox("Follow Mode", &isFollowMode_);

        if (isFollowMode_) {
            // フォロー専用パラメータ
            ImGui::Separator();
            ImGui::Text("Follow Camera Settings");
            ImGui::DragFloat("Follow Distance", &followDistance_, 0.1f, 1.0f, 50.0f);
            ImGui::DragFloat("Follow Height", &followHeight_, 0.1f, -10.0f, 10.0f);
            ImGui::DragFloat("Follow Smoothness", &followSmoothness_, 0.01f, 0.01f, 1.0f);
            ImGui::DragFloat("Rotation Smoothness", &rotationSmoothness_, 0.01f, 0.01f, 1.0f);
        } else {
            // 従来のカメラパラメータ
            float xRotate = baseRotateOffsetX_;
            ImGui::DragFloat("RotateOffsetX(Degree)", &xRotate, 0.01f);
            baseRotateOffsetX_ = ToRadian(xRotate);
            ImGui::DragFloat3("firstOffset", &offset_.x, 0.01f);
        }

        /// セーブとロード
        globalParameter_->ParamSaveForImGui(groupName_);
        globalParameter_->ParamLoadForImGui(groupName_);

        ImGui::PopID();
    }
#endif // _DEBUG
}

// ================================= その他のメソッド ================================= //

void GameCamera::GetIsCameraMove() {
    if ((stickInput_).Length() > 0.1f) {
        // カメラが動いている処理
    }
}

Vector3 GameCamera::GetWorldPos() const {
    return viewProjection_.GetWorldPos();
}

Vector3 GameCamera::GetTargetPos() const {
    return Vector3();
}

void GameCamera::SetTarget(const WorldTransform* target) {
    target_ = target;
    Reset();
}

void GameCamera::Debug() {

    ImGui::DragFloat("rotate", &baseRotateOffsetX_, 0.01f);
    ImGui::DragFloat3("offset", &offset_.x, 0.1f);

    // フォローカメラのデバッグ情報
    if (isFollowMode_) {
        ImGui::Separator();
        ImGui::Text("Follow Camera Debug");
        ImGui::DragFloat("Follow Distance", &followDistance_, 0.1f);
        ImGui::DragFloat("Follow Height", &followHeight_, 0.1f);
        ImGui::DragFloat("Follow Smoothness", &followSmoothness_, 0.01f);
        ImGui::DragFloat("Rotation Smoothness", &rotationSmoothness_, 0.01f);

        if (target_) {
            Vector3 playerPos = target_->GetWorldPos();
            Vector3 cameraPos = GetWorldPos();
            float distance    = (cameraPos - playerPos).Length();
            ImGui::Text("Actual Distance: %.2f", distance);
        }
    }
}

void GameCamera::PlayAnimation(const std::string& filename) {
    rendition_->AnimationPlay(filename);
}

void GameCamera::PlayShake(const std::string& filename) {
    rendition_->ShakePlay(filename);
}