#pragma once
#include "2d/Sprite.h"
#include "3d/ViewProjection.h"
#include "Details/Faction.h"
#include "Vector3.h"

#include <memory>
#include <variant>
#include <vector>

// 前方宣言
class EnemyNPC;
class BoundaryBreaker;

// サポートされる型をvariantで管理
using LockOnVariant = std::variant<EnemyNPC*, BoundaryBreaker*>;

class LockOn {
public:
    LockOn()  = default;
    ~LockOn() = default;

    void Init();
    void Update(const std::vector<LockOnVariant>& targets, const ViewProjection& viewProjection, FactionType playerFaction);
    void Draw();

    bool ExistTarget() const { return currentTarget_.has_value(); }
    Vector3 GetTargetPosition() const;
    void OnObjectDestroyed(const LockOnVariant& obj);

private:
    // ヘルパー関数
    Vector3 GetPosition(const LockOnVariant& target) const;
    bool IsDead(const LockOnVariant& target) const;
    FactionType GetFaction(const LockOnVariant& target) const;
    bool IsLockable(const LockOnVariant& target, FactionType playerFaction) const;

    void Search(const std::vector<LockOnVariant>& targets, const ViewProjection& viewProjection, FactionType playerFaction);
    bool IsTargetRange(const LockOnVariant& target, const ViewProjection& viewProjection, Vector3& positionView);
    void LerpTimeIncrement(float incrementTime);

private:
    std::unique_ptr<Sprite> lockOnMark_;
    std::optional<LockOnVariant> currentTarget_;

    float lerpTime_ = 0.0f;
    Vector2 prePos_;
    Vector2 lockOnMarkPos_;
    float spriteRotation_ = 0.0f;

    // パラメータ
    float minDistance_;
    float maxDistance_;
    float angleRange_;
};