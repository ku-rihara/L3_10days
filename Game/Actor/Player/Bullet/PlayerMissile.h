#pragma once
#include "BaseObject/BaseObject.h"
#include "BasePlayerBullet.h"

class PlayerMissile : public BasePlayerBullet {
public:
    PlayerMissile()  = default;
    ~PlayerMissile() = default;

    // 初期化・更新
    void Init();
    void Update() override;

    void Fire(const Vector3& position, const Vector3& direction, const Quaternion& rotation) override;
    void Deactivate() override;
    Vector3 GetPosition() const override;

    // ミサイル特有の機能
    void SetTarget(const Vector3& targetPosition);
    void ClearTarget();

private:
    // ミサイル特有の更新処理
    void UpdateMissileMovement(float deltaTime);
    void UpdateTargetTracking(float deltaTime);

private:
    float currentLifeTime_ = 0.0f;
    Vector3 velocity_;

    // ミサイル特有のパラメータ
    Vector3 targetPosition_;
    bool hasTarget_         = false;
    float trackingStrength_ = 2.0f;
    float maxTurnRate_      = 3.0f;
};