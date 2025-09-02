#pragma once
#include "BaseObject/BaseObject.h"
#include "utility/ParameterEditor/GlobalParameter.h"
#include <cstdint>

class Player : public BaseObject {
public:
    Player()  = default;
    ~Player() = default;

    // 初期化、更新
    void Init();
    void Update();

    // Move
    void Move();
    void HandleInput(); // 入力処理
    void UpdatePhysics(); // 物理更新

    void DirectionToCamera();

    // editor
    void AdjustParam();
    void BindParams();

    // ベクトル取得
    Vector3 GetForwardVector() const;
    Vector3 GetRightVector() const;
    Vector3 GetUpVector() const;

private:
    // viewProjection
    const ViewProjection* viewProjection_ = nullptr;

    // globalParameter
    GlobalParameter* globalParameter_;
    const std::string groupName_ = "Player";

    // Parameter
    int32_t hp_;
    float speed_;

    // スピードパラメータ
    float forwardSpeed_;
    float pitchSpeed_;
    float yawSpeed_;
    float rollSpeed_;

    // 物理パラメータ
    Vector3 velocity_;
    Vector3 angularVelocity_;
    Vector3 angleInput_;
    Quaternion targetRotation_;

    float rotationSmoothness_;
    float rollRotateLimit_;

    Vector3 direction_;
    float objectiveAngle_;

    float pitchBackTime_;
    float rollBackTime_;
    float pitchReturnThreshold_;

     float rollSideFactor_ = 1.5f;
    float rollDownFactor_ = 0.5f; 

public:
    // ゲッター
    Vector3 GetPosition() const { return baseTransform_.translation_; }
    Vector3 GetRotation() const { return baseTransform_.rotation_; }
    Vector3 GetVelocity() const { return velocity_; }
    float GetSpeed() const { return velocity_.Length(); }

    void SetViewProjection(const ViewProjection* viewProjection) { viewProjection_ = viewProjection; }
};