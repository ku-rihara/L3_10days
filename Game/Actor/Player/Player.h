#pragma once
#include "BaseObject/BaseObject.h"
#include "Behavior/BasePlayerSpeedBehavior.h"
#include "Bullet/PlayerBulletShooter.h"
#include "Easing/Easing.h"
#include "utility/ParameterEditor/GlobalParameter.h"
#include <cstdint>
#include <memory>

class Player : public BaseObject {
public:
    struct SpeedParam {
        float startForwardSpeed;
        float minForwardSpeed;
        float brakeForwardSpeed;
        float maxForwardSpeed;
        float currentForwardSpeed;
        float pitchSpeed;
        float yawSpeed;
        float rollSpeed;
    };

public:
    Player()  = default;
    ~Player() = default;

    // 初期化、更新
    void Init();
    void Update();

    // Move
    void HandleInput();
    void RotateUpdate();

    // speed
    void SpeedInit();
    void SpeedUpdate();

    void CorrectionHorizon();

    // editor
    void AdjustParam();
    void BindParams();

    // ベクトル取得
    Vector3 GetForwardVector() const;
    Vector3 GetRightVector() const;
    Vector3 GetUpVector() const;

    // ロール所得
    float GetRollDegree() const;

   

    bool GetIsUpsideDown()const;

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

    // globalParameter
    GlobalParameter* globalParameter_;
    const std::string groupName_ = "Player";

    // Parameter
    int32_t hp_;

    // speed
    Easing<float> speedEase_;
    SpeedParam speedParam_;

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

public:
    // ゲッター
    const Vector3& GetPosition() const { return baseTransform_.translation_; }
    const Quaternion& GetBaseTQuaternion() const { return baseTransform_.quaternion_; }
    PlayerBulletShooter* GetBulletShooter() const { return bulletShooter_.get(); }
    const SpeedParam& GetSpeedParam() const { return speedParam_; }
    const float& GetRollRotateLimit() const { return rollRotateLimit_; }

    void SetViewProjection(const ViewProjection* viewProjection) { viewProjection_ = viewProjection; }
};