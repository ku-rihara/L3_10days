#pragma once
#include "BaseObject/BaseObject.h"
#include "Behavior/BasePlayerSpeedBehavior.h"
#include "Behavior/PlayerBoost.h"
#include "Behavior/PlayerSpeedDown.h"
#include "Bullet/PlayerBulletShooter.h"
#include "Easing/Easing.h"
#include "utility/ParameterEditor/GlobalParameter.h"
#include <cstdint>
#include <memory>

class Player : public BaseObject {
public:
    Player()  = default;
    ~Player() = default;

    // 初期化、更新
    void Init();
    void Update();

    // Move
    void HandleInput(); // 入力処理
    void RotateUpdate(); // 物理更新

    void SpeedChange();

    void DirectionToCamera();

    // editor
    void AdjustParam();
    void BindParams();

    // ベクトル取得
    Vector3 GetForwardVector() const;
    Vector3 GetRightVector() const;
    Vector3 GetUpVector() const;

    // Behavior management
    void ChangeSpeedBehavior(std::unique_ptr<BasePlayerSpeedBehavior> behavior);
    void UpdateSpeedBehavior();

private:
    // viewProjection
    const ViewProjection* viewProjection_ = nullptr;
    std::unique_ptr<PlayerBulletShooter> bulletShooter_;
    std::unique_ptr<BasePlayerSpeedBehavior> speedBehavior_;

    // Behavior instances
    std::unique_ptr<PlayerBoost> boostBehavior_;
    std::unique_ptr<PlayerSpeedDown> speedDownBehavior_;

    // Button state tracking
    bool isLBPressed_;
    bool wasLBPressed_;

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

    Easing<float> speedChangeEase_;

    float sideFactor_;
    float downFactor_;

public:
    // ゲッター
    const Vector3& GetPosition() const { return baseTransform_.translation_; }
    const Vector3& GetRotation() const { return baseTransform_.rotation_; }
    const Vector3& GetVelocity() const { return velocity_; }
    const float& GetSpeed() const { return velocity_.Length(); }
    const Quaternion& GetQuaternion() const { return baseTransform_.quaternion_; }
    PlayerBulletShooter* GetBulletShooter() const { return bulletShooter_.get(); }
    const float& GetForwardSpeed() const { return forwardSpeed_; }

    void SetViewProjection(const ViewProjection* viewProjection) { viewProjection_ = viewProjection; }
   
};