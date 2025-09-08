#pragma once
#include "Actor/Player/LockOn/LockOn.h"
#include "Actor/Player/TargetManager/TargetManager.h"
#include "BasePlayerBullet.h"
#include "PlayerBulletShooter.h"

class PlayerMissile : public BasePlayerBullet {
public:
    PlayerMissile()  = default;
    ~PlayerMissile() = default;

    // 初期化・更新
    void Init();
    void Update() override;

    void Fire(const Player& player, const LockOn::LockOnVariant* target) override;
    void Deactivate() override;
    Vector3 GetPosition() const override;

    // ターゲット
    void SetTarget(const Vector3& targetPosition);
    void ClearTarget();
    void OnCollisionStay([[maybe_unused]] BaseCollider* other) override;

    // パラメータ設定メソッド
    void SetMissileParameters(const MissileParameter& params);

    // IDベースのターゲット設定
    void SetTargetID(TargetID targetId);
    TargetID GetTargetID() const { return targetId_; }

private:
    // 更新処理
    void UpdateMissileMovement(float deltaTime);
    void UpdateTargetTracking(float deltaTime);
    void UpdateSpeed(float deltaTime);

    void HitBoundary() override;

    // ターゲットの有効性をチェック
    bool IsTargetValid() const;
    Vector3 GetTargetWorldPosition() const;

private:
    float currentLifeTime_ = 0.0f;
    Vector3 velocity_;
    float currentSpeed_ = 0.0f;

    Vector3 targetPosition_;
    bool hasTarget_ = false;

    MissileParameter uniqueParam_;
    

    // IDベースのターゲット管理
    TargetID targetId_            = INVALID_TARGET_ID;
    TargetManager* targetManager_ = nullptr;
};