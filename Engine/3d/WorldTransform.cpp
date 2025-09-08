#include "WorldTransform.h"
#include "Animation/Object3DAnimation.h"
#include "Dx/DirectXCommon.h"
#include <assert.h>
#include <numbers>

void WorldTransform::Init() {

    scale_       = {1, 1, 1};
    rotation_    = {};
    translation_ = {};

    //  行列の更新
    UpdateMatrix();
}

void WorldTransform::TransferMatrix() {

}

void WorldTransform::UpdateMatrix() {
    // SRT更新
    UpdateAffineMatrix();

    // JointParent
    if (HasParentJoint()) {
        UpdateMatrixWithJoint();
    }
    // 通常のparent
    else if (parent_) {
        matWorld_ *= parent_->matWorld_;
    }

    // 定数バッファに転送する
    TransferMatrix();
}
void WorldTransform::BillboardUpdateMatrix(const ViewProjection& viewProjection, const BillboardType& billboardAxis, const AdaptRotate& adaptRotate) {
    // スケール、回転、平行移動行列を計算
    Matrix4x4 scaleMatrix     = MakeScaleMatrix(scale_);
    Matrix4x4 translateMatrix = MakeTranslateMatrix(translation_);

    // カメラ行列を取得
    Matrix4x4 cameraMatrix = viewProjection.GetCameraMatrix();

    // カメラ位置を取得し、オブジェクトとのベクトルを計算
    Vector3 cameraPos = viewProjection.GetWorldPos();
    Vector3 toCamera  = {cameraPos.x - translation_.x, 0.0f, cameraPos.z - translation_.z};
    toCamera          = (toCamera).Normalize();

    // ビルボード行列の計算
    switch (billboardAxis) {
    case BillboardType::XYZ:
        // 全ビルボード
        billboardMatrix_ = cameraMatrix;
        break;

    case BillboardType::Y: {
        // Y軸ビルボード
        float angleY     = std::atan2(toCamera.x, toCamera.z);
        billboardMatrix_ = MakeRotateYMatrix(angleY);
        break;
    }

    default:
        // 何もしない
        billboardMatrix_ = MakeIdentity4x4();
        break;
    }

    if (!adaptRotate.isX) {
        rotation_.x = 0.0f;
    }
    if (!adaptRotate.isY) {
        rotation_.y = 0.0f;
    }
    if (!adaptRotate.isZ) {
        rotation_.z = 0.0f;
    }

    // X/Z軸の回転を適用
    Matrix4x4 xzRotationMatrix = MakeRotateMatrix(rotation_);

    // ビルボード行列計算
    billboardMatrix_ = xzRotationMatrix * billboardMatrix_;

    // 平行移動成分をクリア
    billboardMatrix_.m[3][0] = 0.0f;
    billboardMatrix_.m[3][1] = 0.0f;
    billboardMatrix_.m[3][2] = 0.0f;

    // ワールド行列を計算
    matWorld_ = scaleMatrix * billboardMatrix_ * translateMatrix;

    // 親の処理を修正
    if (HasParentJoint()) {
        UpdateMatrixWithJoint();
    }
    // 通常のparent - Billboardの場合は位置のみ適用
    else if (parent_) {
        // 親のワールド位置を取得
        Vector3 parentWorldPos = parent_->GetWorldPos();

        // 現在のワールド位置に親の位置を加算
        matWorld_.m[3][0] += parentWorldPos.x;
        matWorld_.m[3][1] += parentWorldPos.y;
        matWorld_.m[3][2] += parentWorldPos.z;
    }

    // 定数バッファに転送する
    TransferMatrix();
}
void WorldTransform::SetParent(const WorldTransform* parent) {
    parent_ = parent;
}

Vector3 WorldTransform::LookAt(const Vector3& direction) const {
    // 引数の方向ベクトルを正規化
    Vector3 normalizedDirection = Vector3::Normalize(direction);

    // 現在の行列を基にワールド方向を計算
    Matrix4x4 rotateMatrix = MakeRotateMatrix(rotation_);
    Vector3 worldDirection = TransformNormal(normalizedDirection, rotateMatrix);

    // 正規化して返す
    return Vector3::Normalize(worldDirection);
}

///=====================================================
/// ローカル座標取得
///=====================================================
Vector3 WorldTransform::GetLocalPos() const {
    //
    if (parent_ == nullptr) {
        return GetWorldPos();
    }

    // 親のワールド行列の逆行列を計算
    Matrix4x4 parentInverse = Inverse(parent_->matWorld_);

    // ワールド座標を取得
    Vector3 worldPos = GetWorldPos();

    // 親の逆行列を使ってローカル座標を計算
    Vector3 localPos = TransformMatrix(worldPos, parentInverse);
    return localPos;
}

void WorldTransform::UpdateAffineMatrix() {
    switch (rotateOder_) {
    case RotateOder::XYZ:
        matWorld_ = MakeAffineMatrix(scale_, rotation_, translation_);
        break;
    case RotateOder::Quaternion:
        quaternion_.Normalize();
        matWorld_ = MakeAffineMatrixQuaternion(scale_, quaternion_, translation_);

        break;
    default:
        break;
    }
}

void WorldTransform::SetParentJoint(const Object3DAnimation* animation, const std::string& jointName) {
    if (!animation || jointName.empty()) {
        ClearParentJoint();
        return;
    }

    const auto& skeleton = animation->GetSkeleton();

    // Joint名からインデックスを検索
    auto it = skeleton.jointMap.find(jointName);
    if (it == skeleton.jointMap.end()) {
        ClearParentJoint();
        return;
    }

    parentAnimation_  = animation;
    parentJointIndex_ = it->second;
    parentJointName_  = skeleton.joints[parentJointIndex_].name;

    parent_ = nullptr;
}

///=====================================================
/// WorldPos取得
///=====================================================
Vector3 WorldTransform::GetWorldPos() const {

    return Vector3(
        matWorld_.m[3][0], // X成分
        matWorld_.m[3][1], // Y成分
        matWorld_.m[3][2]  // Z成分
    );
}

void WorldTransform::ClearParentJoint() {
    parentAnimation_  = nullptr;
    parentJointIndex_ = -1;
    parentJointName_.clear();

    // 行列を更新
    UpdateMatrix();
}

void WorldTransform::UpdateMatrixWithJoint() {
    if (!HasParentJoint()) {
        return;
    }

    const auto& skeleton = parentAnimation_->GetSkeleton();

    //
    if (parentJointIndex_ < 0 || parentJointIndex_ >= static_cast<int32_t>(skeleton.joints.size())) {

        return;
    }

    // 親JointのskeletonSpaceMatrix取得
    const Joint& parentJoint    = skeleton.joints[parentJointIndex_];
    Matrix4x4 parentJointMatrix = parentJoint.skeletonSpaceMatrix;

    // WorldTransform
    Matrix4x4 animationWorldMatrix = parentAnimation_->transform_.matWorld_;
    parentJointMatrix              = parentJointMatrix * animationWorldMatrix;

    matWorld_ *= parentJointMatrix;
}

bool WorldTransform::HasParentJoint() const {
    return parentAnimation_ != nullptr && parentJointIndex_ != -1;
}