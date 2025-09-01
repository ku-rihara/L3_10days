#pragma once
#include "3d/ViewProjection.h"
#include "3d/WorldTransform.h"
#include "CameraRendition.h"
#include "utility/ParameterEditor/GlobalParameter.h"
#include <memory>

class GameCamera {
private:
    /// ===================================================
    /// private variable
    /// ===================================================

    GlobalParameter* globalParameter_;
    const std::string groupName_ = "GameCamera";

    ViewProjection viewProjection_;

    /// 演出管理クラス
    std::unique_ptr<CameraRendition> rendition_;

    const WorldTransform* target_ = nullptr;
    Vector3 stickInput_;
    Vector3 interTarget_ = {};
    float destinationAngleY_;
    int viewMoveTime_;

    Vector3 shakeOffsetPos_;
    float rotate_;

    // オフセット
    Vector3 offset_;
    float baseRotateOffsetX_;

    // 背後追従カメラ用パラメータ
    bool isFollowMode_;
    float followDistance_; // プレイヤーからの距離
    float followHeight_; // プレイヤーからの高さオフセット
    float followSmoothness_; // カメラ位置の補間速度
    float rotationSmoothness_; // カメラ回転の補間速度

public:
    /// ===================================================
    /// public method
    /// ===================================================
    void Init(); /// 初期化
    void Update(); /// 更新
    void MoveUpdate(); /// カメラ移動更新
    void FollowUpdate(); /// 背後追従更新
    void Reset(); /// リセット
    void GetIsCameraMove(); /// カメラ動いているか判定

    void Debug();

    void RotateAdapt();
    void TranslateAdapt();
    void ToggleFollowMode(); /// フォローモード切り替え

    Vector3 OffsetCalc(const Vector3& offset) const;
    Vector3 GetWorldPos() const;
    Vector3 CalculateLookAtRotation(const Vector3& direction, const Vector3& up); /// LookAt回転計算

    void PlayAnimation(const std::string& filename);
    void PlayShake(const std::string& filename);

    ///-------------------------------------------------------------------------------------
    /// Editor
    ///------------------------------------------------------------------------------------
    void BindParams();
    void AdjustParam();

    /// ===================================================
    /// getter
    /// ===================================================
    const ViewProjection& GetViewProjection() { return viewProjection_; }
    ViewProjection& GetViewProjectionRef() { return viewProjection_; }
    Vector3 GetTargetPos() const;
    Vector3 GetOffset() const { return offset_; };
    bool IsFollowMode() const { return isFollowMode_; }
    float GetFollowDistance() const { return followDistance_; }
    float GetFollowHeight() const { return followHeight_; }

    /// ===================================================
    /// setter
    /// ===================================================
    void SetTarget(const WorldTransform* target);
    void SetRotate(const float& rotate) { rotate_ = rotate; }
    void SetOffSet(const Vector3& offset) { offset_ = offset; }
    void SetShakePos(const Vector3& shake) { shakeOffsetPos_ = shake; }
    void SetShakePosY(const float& shake) { shakeOffsetPos_.y = shake; }
    void SetDestinationAngleY_(float angle) { destinationAngleY_ = angle; }
    void SetViewProjectionPos(Vector3 pos) { viewProjection_.translation_ = pos; }
    void SetFollowMode(bool followMode) { isFollowMode_ = followMode; }
    void SetFollowDistance(float distance) { followDistance_ = distance; }
    void SetFollowHeight(float height) { followHeight_ = height; }
    void SetFollowSmoothness(float smoothness) { followSmoothness_ = smoothness; }
    void SetRotationSmoothness(float smoothness) { rotationSmoothness_ = smoothness; }
};