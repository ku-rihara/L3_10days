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

public:
    /// ===================================================
    /// public method
    /// ===================================================
    void Init(); /// 初期化
    void Update(); /// 更新
    void MoveUpdate(); /// カメラ移動更新
    void Reset(); /// リセット
    void GetIsCameraMove(); /// カメラ動いているか判定

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

    /// ===================================================
    /// getter
    /// ===================================================
    const ViewProjection& GetViewProjection() { return viewProjection_; }
    ViewProjection& GetViewProjectionRef() { return viewProjection_; }
    Vector3 GetTargetPos() const;
    Vector3 GetOffset() const { return offset_; };
   
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
};