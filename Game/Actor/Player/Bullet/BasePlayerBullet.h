#pragma once
#include "Actor/Player/LockOn/LockOn.h"
#include "BaseObject/BaseObject.h"
#include "Collider/AABBCollider.h"

enum class BulletType {
    NORMAL,
    MISSILE,
    COUNT,
};

struct BulletParameter {
    float lifeTime;
    float speed;
    float damage;
};

class Player;
class BasePlayerBullet : public BaseObject, public AABBCollider {
public:
    BasePlayerBullet();
    virtual ~BasePlayerBullet() = default;

    virtual void Init() override;
    virtual void Update()                                                        = 0;
    virtual void Fire(const Player& player, const LockOn::LockOnVariant* target) = 0;

    virtual void Deactivate()           = 0;
    virtual Vector3 GetPosition() const = 0;

protected:
    bool isActive_;
    BulletParameter param_;
    BulletType type_;
    const LockOn::LockOnVariant* target_;
  
public:
    // 既存のゲッター
    bool GetIsActive() const { return isActive_; }
    void SetParameter(const BulletType& type, const BulletParameter& parameter);
};