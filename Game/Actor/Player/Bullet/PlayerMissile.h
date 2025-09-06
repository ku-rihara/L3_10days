#pragma once
#include "BaseObject/BaseObject.h"
#include "BasePlayerBullet.h"

struct MissileParameter;

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

    // ターゲット
    void SetTarget(const Vector3& targetPosition);
    void ClearTarget();

    // パラメータ設定メソッド
    void SetMissileParameters(const MissileParameter& params);

private:
    // ミサイル特有の更新処理
    void UpdateMissileMovement(float deltaTime);
    void UpdateTargetTracking(float deltaTime);

private:
    float currentLifeTime_ = 0.0f;
    Vector3 velocity_;

    Vector3 targetPosition_;
    bool hasTarget_ = false;
    float trackingStrength_;
    float maxTurnRate_;
    bool enableTracking_ = true;
};