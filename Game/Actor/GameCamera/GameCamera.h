#pragma once
#include "3d/ViewProjection.h"
#include "3d/WorldTransform.h"
#include "CameraRendition.h"
#include "utility/ParameterEditor/GlobalParameter.h"
#include <memory>

class Player;
class GameCamera {
    struct OffsetParam {
        Vector3 shakeOffsetPos;
        Vector3 cameraOffset;
        Vector3 rotationOffset;
        float cameraZOffsetMin;
        float cameraZOffsetMax;
    };

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

    void CalcCameraOffset();

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
    Player* pPlayer_;
    OffsetParam offsetParam_;

    /// 演出管理クラス
    std::unique_ptr<CameraRendition> rendition_;

    const WorldTransform* target_ = nullptr;
    Vector3 stickInput_;
    Vector3 interTarget_ = {};
    float destinationAngleY_;
    int viewMoveTime_;
   
    float rotate_;
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
    void SetPlayer(Player* player);
    void SetRotate(const float& rotate) { rotate_ = rotate; }
    void SetDestinationAngleY_(float angle) { destinationAngleY_ = angle; }
    void SetViewProjectionPos(Vector3 pos) { viewProjection_.translation_ = pos; }
};