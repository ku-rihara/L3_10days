#pragma once
#include "BaseObject/BaseObject.h"
#include"Actor/Player/LockOn/lockOn.h"
#include "BasePlayerBullet.h"

class Player;
class LockOn;
class PlayerNormalBullet : public BasePlayerBullet {
public:
    PlayerNormalBullet()  = default;
    ~PlayerNormalBullet() = default;

    // 初期化・更新
    void Init();
    void Update() override;

    void Fire(const Player& player, const LockOn::LockOnVariant* target) override;
    void Deactivate() override;
    Vector3 GetPosition() const override;

    void HitBoundary() override;
    void OnCollisionStay([[maybe_unused]] BaseCollider* other) override;

private:
    // 弾種別ごとの更新処理
    void UpdateNormalBullet(float deltaTime);
   

private:
    float currentLifeTime_ = 0.0f;
    Vector3 velocity_;
};