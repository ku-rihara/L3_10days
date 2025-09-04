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

    void CorrectionHorizon();

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
    // other class
    const ViewProjection* viewProjection_ = nullptr;
    std::unique_ptr<PlayerBulletShooter> bulletShooter_;
    std::unique_ptr<BasePlayerSpeedBehavior> speedBehavior_;

    // ブースト
    bool isLBPressed_;
    bool wasLBPressed_;
    Easing<float> speedChangeEase_;

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
    Vector3 velocity_          = Vector3::ZeroVector();
    Vector3 angularVelocity_   = Vector3::ZeroVector();
    Vector3 angleInput_        = Vector3::ZeroVector();
    Quaternion targetRotation_ = Quaternion::Identity();

    // ピッチ
    float pitchBackTime_;
    float pitchReturnThreshold_;

    // バンク強さ、逆さ判定の値
    float bankRate_;
    float reverseDecisionValue_;


    //  逆さ補正中かのフラグ
    bool isAutoRecovering_ = false;

    // roll
    float targetRoll_;
    float currentRoll_;
    float rollBackTime_;
    float rotationSmoothness_;
    float rollRotateLimit_;
    float currentMaxRoll_;


     float targetRoll_  = 0.0f; // 目標のロール角
    float currentRoll_ = 0.0f; // 実際のロール角
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