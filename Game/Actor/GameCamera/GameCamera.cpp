#include "GameCamera.h"
// Function
#include "Actor/Player/Player.h"
#include "Easing/EasingFunction.h"
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

    globalParameter_ = GlobalParameter::GetInstance();
    globalParameter_->CreateGroup(groupName_, false);
    BindParams();
    globalParameter_->SyncParamForGroup(groupName_);

    rendition_ = std::make_unique<CameraRendition>();
    rendition_->Init();
    rendition_->SetGameCamera(this);
    rendition_->SetViewProjection(&viewProjection_);
}

void GameCamera::Update() {
    rendition_->Update();
    offsetParam_.shakeOffsetPos = rendition_->GetShakeOffset();

    // カメラオフセット計算
    CameraOffsetCalc();

    if (target_) {

        viewProjection_.translation_ = offsetParam_.cameraOffset + offsetParam_.shakeOffsetPos;
        viewProjection_.rotation_    = offsetParam_.rotationOffset;

        // 目標のローカルオフセット位置
        Vector3 targetLocalPos = offsetParam_.cameraOffset + offsetParam_.shakeOffsetPos;
        // position補間
        viewProjection_.translation_ = Lerp(viewProjection_.translation_, targetLocalPos, smoothness_);
    }

    viewProjection_.UpdateMatrix();
}

void GameCamera::Reset() {
    // 追従対象がいれば
    if (target_) {
        // 追従座標・角度の初期化
        interTarget_ = target_->translation_;

        viewProjection_.rotation_.y = LerpShortAngle(viewProjection_.rotation_.y, target_->rotation_.y, 0.3f);
    }
    destinationAngleY_ = viewProjection_.rotation_.y;

    // 追従対象からのオフセット
    Vector3 offset               = OffsetCalc(offsetParam_.cameraOffset);
    viewProjection_.translation_ = interTarget_ + offset;
}

Vector3 GameCamera::OffsetCalc(const Vector3& offset) const {
    // カメラの角度から回転行列を計算する
    Matrix4x4 rotateMatrix = MakeRotateYMatrix(viewProjection_.rotation_.y);
    Vector3 resultOffset   = TransformNormal(offset + offsetParam_.shakeOffsetPos, rotateMatrix);
    return resultOffset;
}

void GameCamera::BindParams() {
    globalParameter_->Bind(groupName_, "rotationOffset", &offsetParam_.rotationOffset);
    globalParameter_->Bind(groupName_, "baseCameraOffset", &offsetParam_.baseOffset);
    globalParameter_->Bind(groupName_, "cameraZOffsetMin", &offsetParam_.cameraZOffsetMin);
    globalParameter_->Bind(groupName_, "cameraZOffsetMax", &offsetParam_.cameraZOffsetMax);
    globalParameter_->Bind(groupName_, "cameraZOffsetMin", &offsetParam_.cameraZOffsetMin);
    globalParameter_->Bind(groupName_, "cameraZOffsetMax", &offsetParam_.cameraZOffsetMax);
    globalParameter_->Bind(groupName_, "cameraXOffsetMin", &offsetParam_.cameraXOffsetMin);
    globalParameter_->Bind(groupName_, "cameraXOffsetMax", &offsetParam_.cameraXOffsetMax);
    globalParameter_->Bind(groupName_, "smoothness", &smoothness_);
    globalParameter_->Bind(groupName_, "cameraXSmoothness", &cameraXSmoothnessInRotating_);
    globalParameter_->Bind(groupName_, "cameraXSmoothnessInNoRotate", &cameraXSmoothnessInNoRotate_);
}
void GameCamera::SetTarget(const WorldTransform* target) {
    target_ = target;

    // ViewProjectionの親を設定
    viewProjection_.SetParent(target);

    // 初期位置をリセット
    viewProjection_.translation_ = offsetParam_.cameraOffset;
    viewProjection_.rotation_    = offsetParam_.rotationOffset;

    Reset();
}

void GameCamera::AdjustParam() {
#ifdef _DEBUG
    if (ImGui::CollapsingHeader(groupName_.c_str())) {
        ImGui::PushID(groupName_.c_str());

        ImGui::Separator();
        ImGui::Text("Parent Camera Settings");
        ImGui::DragFloat3("Camera BaseOffset", &offsetParam_.baseOffset.x, 0.01f);
        ImGui::DragFloat("Camera OffsetZMin", &offsetParam_.cameraZOffsetMin, 0.01f);
        ImGui::DragFloat("Camera OffsetZMax", &offsetParam_.cameraZOffsetMax, 0.01f);
        ImGui::DragFloat("Camera OffsetXMin", &offsetParam_.cameraXOffsetMin, 0.01f);
        ImGui::DragFloat("Camera OffsetXMax", &offsetParam_.cameraXOffsetMax, 0.01f);
        ImGui::DragFloat3("Rotation Offset", &offsetParam_.rotationOffset.x, 0.01f);
        ImGui::DragFloat("Smoothness", &smoothness_, 0.01f, 0.01f, 1.0f);
        ImGui::DragFloat("cameraXSmoothnessInRotate", &cameraXSmoothnessInRotating_, 0.01f);
        ImGui::DragFloat("cameraXSmoothnessInNoRotate", &cameraXSmoothnessInNoRotate_, 0.01f, 0.01f, 1.0f);

        globalParameter_->ParamSaveForImGui(groupName_);
        globalParameter_->ParamLoadForImGui(groupName_);

        ImGui::PopID();
    }

    rendition_->Edit();
#endif
}

// ================================= カメラオフセット計算 ================================= //

void GameCamera::OffsetInvRangeCalc() {

    float zMax = pPlayer_->GetSpeedParam().maxForwardSpeed;
    float zMin = pPlayer_->GetSpeedParam().brakeForwardSpeed;

    float xMax = pPlayer_->GetRollRotateLimit();
    float xMin = -pPlayer_->GetRollRotateLimit();

    offsetParam_.offsetZInvRange = 1.0f / (zMax - zMin);
    offsetParam_.offsetXInvRange = 1.0f / (xMax - xMin);
}

void GameCamera::CameraOffsetCalc() {

    // Camera Z
    float zMin            = pPlayer_->GetSpeedParam().minForwardSpeed;
    float zOffsetInvRange = offsetParam_.offsetZInvRange;
    float zT              = (pPlayer_->GetSpeedParam().currentForwardSpeed - zMin) * zOffsetInvRange;

    // Camera X
    float xMin            = -pPlayer_->GetRollRotateLimit();
    float xOffsetInvRange = offsetParam_.offsetXInvRange;
    float currentX        = pPlayer_->GetRollDegree();
    float xT              = (currentX - xMin) * xOffsetInvRange;

    // 目標のカメラオフセットを計算
    float targetCameraOffsetX;

    if (pPlayer_->CheckIsRollMax()) {
        // ロール最大時
        currentCameraXSmoothness_ = cameraXSmoothnessInRotating_;
        targetCameraOffsetX = offsetParam_.baseOffset.x - Lerp(offsetParam_.cameraXOffsetMin, offsetParam_.cameraXOffsetMax, xT);
    } else {
        // ロール最大でない時
        currentCameraXSmoothness_ = cameraXSmoothnessInNoRotate_;
        targetCameraOffsetX = offsetParam_.baseOffset.x;
    }

    // 滑らかに補間
    offsetParam_.cameraOffset.x = EaseInSine(offsetParam_.cameraOffset.x, targetCameraOffsetX, currentCameraXSmoothness_, 1.0f);

    offsetParam_.cameraOffset.y = offsetParam_.baseOffset.y;
    offsetParam_.cameraOffset.z = offsetParam_.baseOffset.z + Lerp(offsetParam_.cameraZOffsetMin, offsetParam_.cameraZOffsetMax, zT);
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
    return target_->GetWorldPos();
}

void GameCamera::Debug() {
}

void GameCamera::PlayAnimation(const std::string& filename) {
    rendition_->AnimationPlay(filename);
}

void GameCamera::PlayShake(const std::string& filename) {
    rendition_->ShakePlay(filename);
}

void GameCamera::SetPlayer(Player* player) {
    pPlayer_ = player;
    OffsetInvRangeCalc();
}