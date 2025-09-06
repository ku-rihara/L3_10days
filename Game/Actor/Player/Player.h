#pragma once

#include "Actor/NPC/Navigation/MoveConstraint.h"
#include "BaseObject/BaseObject.h"
#include "Behavior/BasePlayerSpeedBehavior.h"
#include "Bullet/PlayerBulletShooter.h"
#include "Easing/Easing.h"
#include "Reticle/PlayerReticle.h"
#include "utility/ParameterEditor/GlobalParameter.h"
#include <cstdint>
#include <memory>
#include <vector>

class GameCamera;
class Boundary;
struct Hole;
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
        float autoRotateSpeed;
    };

    struct BoundaryHoleSource : IHoleSource {
        const Boundary* boundary = nullptr;
        const std::vector<Hole>& GetHoles() const override;
    };

public:
    Player()  = default;
    ~Player() = default;

    // 初期化、更新
    void Init();
    void Update();

    void ReticleDraw();

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

    // 逆さ判定
    bool GetIsUpsideDown();

    // Behavior management
    void ChangeSpeedBehavior(std::unique_ptr<BasePlayerSpeedBehavior> behavior);
    void UpdateSpeedBehavior();

private:
    // Move
    void HandleInput();
    void RotateUpdate();
    void MoveUpdate();
    void ReboundByBoundary();

private:
    // ブースト
    bool isLBPressed_;
    bool wasLBPressed_;

    // globalParameter
    GlobalParameter* globalParameter_;
    const std::string groupName_ = "Player";
    GameCamera* pGameCamera_      = nullptr;

    // Parameter
    int32_t hp_;

    // speed
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
    float upDot_;

    // 境界反発
    bool isRebound_ = false;
    float reboundPower_;
    Vector3 reboundVelocity_;
    float reboundDecay_;
    float minReboundVelocity_;
    Vector3 lastCollisionNormal_ = Vector3::ZeroVector();

    // 補正時の自動操作
    bool isAutoRotateByCollision = false;
    float autoRotateDirection_   = 0.0f; 
    float autoUnLockTime_;
  
    // roll
    float targetRoll_;
    float currentRoll_;
    float rollBackTime_;
    float rotationSmoothness_;
    float rollRotateLimit_;
    float currentMaxRoll_;

    // other class
    const ViewProjection* viewProjection_                   = nullptr;
    std::unique_ptr<PlayerReticle> reticle_                 = nullptr;
    std::unique_ptr<PlayerBulletShooter> bulletShooter_     = nullptr;
    std::unique_ptr<BasePlayerSpeedBehavior> speedBehavior_ = nullptr;
    std::unique_ptr<IMoveConstraint> moveConstraint_        = nullptr;

    BoundaryHoleSource holeSource_;

public:
    // ゲッター
    const Vector3& GetPosition() const { return baseTransform_.translation_; }
    const Quaternion& GetBaseQuaternion() const { return baseTransform_.quaternion_; }
    const Quaternion& GetObjQuaternion() const { return obj3d_->transform_.quaternion_; }
    PlayerBulletShooter* GetBulletShooter() const { return bulletShooter_.get(); }
    const SpeedParam& GetSpeedParam() const { return speedParam_; }
    const float& GetRollRotateLimit() const { return rollRotateLimit_; }

    void SetGameCamera(GameCamera* camera);
    void SetViewProjection(const ViewProjection* viewProjection) { viewProjection_ = viewProjection; }
};