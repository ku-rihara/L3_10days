#pragma once
#include "3d/ViewProjection.h"
#include "3d/WorldTransform.h"
#include "CameraRendition.h"
#include "utility/ParameterEditor/GlobalParameter.h"
#include <memory>

class GameCamera {

public:
    GameCamera()  = default;
    ~GameCamera() = default;
    /// ===================================================
    /// public method
    /// ===================================================
    void Init();
    void Update();
    void Reset();
    void GetIsCameraMove();

    void Debug();

    void RotateAdapt();
    void TranslateAdapt();

    Vector3 OffsetCalc(const Vector3& offset) const;
    Vector3 GetWorldPos() const;

    void PlayAnimation(const std::string& filename);
    void PlayShake(const std::string& filename);

    ///-------------------------------------------------------------------------------------
    /// Editor
    ///------------------------------------------------------------------------------------
    void BindParams();
    void AdjustParam();

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

    Vector3 cameraOffset_;
    Vector3 rotationOffset_;
    float smoothness_;
  
public:
    /// ===================================================
    /// getter
    /// ===================================================
    const ViewProjection& GetViewProjection() { return viewProjection_; }
    ViewProjection& GetViewProjectionRef() { return viewProjection_; }
    Vector3 GetTargetPos() const;
   
    /// ===================================================
    /// setter
    /// ===================================================
    void SetTarget(const WorldTransform* target);
    void SetRotate(const float& rotate) { rotate_ = rotate; }
    void SetShakePos(const Vector3& shake) { shakeOffsetPos_ = shake; }
    void SetShakePosY(const float& shake) { shakeOffsetPos_.y = shake; }
    void SetDestinationAngleY_(float angle) { destinationAngleY_ = angle; }
    void SetViewProjectionPos(Vector3 pos) { viewProjection_.translation_ = pos; }
};