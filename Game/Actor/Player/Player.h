#pragma once

#include "Actor/NPC/Navigation/MoveConstraint.h"
#include "BaseObject/BaseObject.h"
#include "Behavior/BasePlayerSpeedBehavior.h"
#include "Bullet/PlayerBulletShooter.h"
#include "Reticle/PlayerReticle.h"
#include "utility/ParameterEditor/GlobalParameter.h"
// parts
#include "Parts/PlayerBackWing.h"
#include "Parts/PlayerBackWingCenter.h"
#include "Parts/PlayerFrontWing.h"
// UI
#include "UI/PlayerLifeUI.h"
#include"UI/MissileIconUI.h"
#include"UI/DMGTextUI.h"
#include"UI/PlayerDamageParUI.h"

#include <array>
#include <cstdint>
#include <memory>
#include <vector>

class LockOn;
class GameCamera;
class Boundary;
struct Hole;
class Player : public BaseObject,public AABBCollider {
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

    struct CollisionParamInfo {
        Vector3 collisionSize;
        float currentCollTime;
        float coolTime;
        bool isColliding;
        float damageValue;
    };

    struct AutoCorrectionParam {
        bool isAutoRotate;
        float autoRotateDirection_;
        float currentAutoSpeed_;
        float autoRotateSpeed_;
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
    void PartsInit();
    void UIInit();

    void Update();
    void PartsUpdate();
    void UIUpdate();

    void UIDraw();
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

    // Check
    void CheckIsUpsideDown(); //<逆さ判定
    bool CheckIsRollMax() const; //<ロールMAX判定

    // Behavior management
    void ChangeSpeedBehavior(std::unique_ptr<BasePlayerSpeedBehavior> behavior);
    void UpdateSpeedBehavior();

	/// ポーズを閉じたときに呼ばれる
    void ClosedPaused();
    void TakeDamageForBoundary();

    // collision
    void OnCollisionStay([[maybe_unused]] BaseCollider* other);
    Vector3 GetCollisionPos() const override;
    void CollisionCollingUpdate();

private:
    // Move
    void HandleInput();
    void RotateUpdate();
    void MoveUpdate();
    void ReboundByBoundary();

	void ReadJsonInversePitch();

private:
    // ブースト
    bool isLBPressed_;
    bool wasLBPressed_;

    // Parts
    std::array<std::unique_ptr<PlayerBackWing>, 2> backWings_;
    std::array<std::unique_ptr<PlayerFrontWing>, 2> frontWings_;
    std::unique_ptr<PlayerBackWingCenter> backWingCenter_ = nullptr;

    // UIs
    std::unique_ptr<PlayerLifeUI> lifeUI_ = nullptr;
    std::unique_ptr<DMGTextUI> dmgTextUI_ = nullptr;
    std::unique_ptr<PlayerDamageParUI> dmgParUI_ = nullptr;
    std::array<std::unique_ptr<MissileIconUI>, 2> missileUIs_;

    // globalParameter
    GlobalParameter* globalParameter_;
    const std::string groupName_ = "Player";
    GameCamera* pGameCamera_     = nullptr;

    // collisionInfo
    CollisionParamInfo collisionParamInfo_;

    // Parameter
    float hp_;
    float maxHp_;
    float damageValueByBoundary_;

    // speed
    SpeedParam speedParam_;

    // 物理パラメータ
    Vector3 velocity_          = Vector3::ZeroVector();
    Vector3 angularVelocity_   = Vector3::ZeroVector();
    Vector3 angleInput_        = Vector3::ZeroVector();
    Quaternion targetRotation_ = Quaternion::Identity();

    Vector3 startPos_;

    // ピッチ
    float pitchBackTime_;
    float pitchReturnThreshold_;
	bool inversePitch_ = false;

    // バンク強さ、逆さ判定の値
    float bankRate_;
    float reverseDecisionValue_;

    // 境界反発
    bool isRebound_ = false;
    float reboundPower_;
    Vector3 reboundVelocity_;
    float reboundDecay_;
    float minReboundVelocity_;
    Vector3 lastCollisionNormal_ = Vector3::ZeroVector();

    // 補正時の自動操作
    AutoCorrectionParam reboundCorrectionParam_;
    AutoCorrectionParam invCorrectionParam_;

    float upDot_;
    bool isUpsideDown_;

    // roll
    float targetRoll_;
    float currentRoll_;
    float rollBackTime_;
    float rotationSmoothness_;
    float rollRotateLimit_;
    float rollRotateOffset_;
    float currentMaxRoll_;
    float rollInput_;

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
    const Vector3& GetAngleInput() const { return angleInput_; }
    float GetHP() const { return hp_; }
    const float& GetMaxHP() const { return maxHp_; }
	float GetRollInput() const { return rollInput_; }
	bool GetInversePitch() const { return inversePitch_; }

    void SetGameCamera(GameCamera* camera);
    void SetLockOn(LockOn* lockOn);
    void SetViewProjection(const ViewProjection* viewProjection);
    void SetHP(float hp) { hp_ = hp; }
	void SetInversePitch(bool inv) { inversePitch_ = inv; }
};