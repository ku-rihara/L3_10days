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

    globalParameter_ = GlobalParameter::GetInstance();
    globalParameter_->CreateGroup(groupName_, false);
    BindParams();
    globalParameter_->SyncParamForGroup(groupName_);

    rendition_ = std::make_unique<CameraRendition>();
    rendition_->Init();
    rendition_->SetGameCamera(this);
    rendition_->SetViewProjection(&viewProjection_);

    smoothness_ = 0.1f;
}

void GameCamera::Update() {
    rendition_->Update();
    shakeOffsetPos_ = rendition_->GetShakeOffset();

    if (target_) {

        // 目標のローカルオフセット位置
        Vector3 targetLocalPos = cameraOffset_ + shakeOffsetPos_;
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
    Vector3 offset               = OffsetCalc(offset_);
    viewProjection_.translation_ = interTarget_ + offset;
}

Vector3 GameCamera::OffsetCalc(const Vector3& offset) const {
    // カメラの角度から回転行列を計算する
    Matrix4x4 rotateMatrix = MakeRotateYMatrix(viewProjection_.rotation_.y);
    Vector3 resultOffset   = TransformNormal(offset + shakeOffsetPos_, rotateMatrix);
    return resultOffset;
}

void GameCamera::BindParams() {
    globalParameter_->Bind(groupName_, "rotationOffset", &rotationOffset_);
    globalParameter_->Bind(groupName_, "cameraOffset", &cameraOffset_);
    globalParameter_->Bind(groupName_, "rollFollowFactor", &rollFollowFactor_);
    globalParameter_->Bind(groupName_, "rollSmoothness", &rollSmoothness_);
}

void GameCamera::SetTarget(const WorldTransform* target) {
    target_ = target;

    // ViewProjectionの親を設定
    viewProjection_.SetParent(target);

    // 初期位置をリセット
    viewProjection_.ignoreParentRoll_ = false;
    viewProjection_.translation_      = cameraOffset_;
    viewProjection_.rotation_         = rotationOffset_;

    Reset();
}

void GameCamera::AdjustParam() {
#ifdef _DEBUG
    if (ImGui::CollapsingHeader(groupName_.c_str())) {
        ImGui::PushID(groupName_.c_str());

        ImGui::Separator();
        ImGui::Text("Parent Camera Settings");
        ImGui::DragFloat3("Camera Offset", &cameraOffset_.x, 0.1f);
        ImGui::DragFloat3("Rotation Offset", &rotationOffset_.x, 0.01f);
        ImGui::DragFloat("Smoothness", &smoothness_, 0.01f, 0.01f, 1.0f);

        ImGui::Separator();
        ImGui::Text("Roll Follow Settings");
        ImGui::DragFloat("Roll Follow Factor", &rollFollowFactor_, 0.01f, 0.0f, 1.0f);
        ImGui::DragFloat("Roll Smoothness", &rollSmoothness_, 0.01f, 0.01f, 1.0f);

        if (ImGui::Button("Apply Offset")) {
            if (target_) {
                viewProjection_.translation_ = cameraOffset_;
                viewProjection_.rotation_    = rotationOffset_;
            }
        }

        globalParameter_->ParamSaveForImGui(groupName_);
        globalParameter_->ParamLoadForImGui(groupName_);

        ImGui::PopID();
    }
#endif
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

void GameCamera::Debug() {

    ImGui::DragFloat("rotate", &baseRotateOffsetX_, 0.01f);
    ImGui::DragFloat3("offset", &offset_.x, 0.1f);
}

void GameCamera::PlayAnimation(const std::string& filename) {
    rendition_->AnimationPlay(filename);
}

void GameCamera::PlayShake(const std::string& filename) {
    rendition_->ShakePlay(filename);
}