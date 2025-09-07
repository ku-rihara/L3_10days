#include "TargetManager.h"
#include"Actor/NPC/BoundaryBreaker/BoundaryBreaker.h"
#include "Actor/NPC/EnemyNPC.h"
#include <algorithm>

TargetManager* TargetManager::GetInstance() {
    static TargetManager instance;
    return &instance;
}

TargetID TargetManager::RegisterTarget(const LockOn::LockOnVariant& target) {
    TargetID id  = nextId_++;
    targets_[id] = target;
    return id;
}

void TargetManager::UnregisterTarget(TargetID targetId) {
    if (targetId != INVALID_TARGET_ID) {
        targets_.erase(targetId);
    }
}

void TargetManager::UnregisterTarget(const LockOn::LockOnVariant& target) {
    // ターゲットに対応するIDを探して削除
    auto it = std::find_if(targets_.begin(), targets_.end(),
        [&target](const auto& pair) {
            return pair.second == target;
        });

    if (it != targets_.end()) {
        targets_.erase(it);
    }
}

std::optional<LockOn::LockOnVariant> TargetManager::GetTarget(TargetID targetId) const {
    auto it = targets_.find(targetId);
    if (it != targets_.end()) {
        return it->second;
    }
    return std::nullopt;
}

Vector3 TargetManager::GetTargetPosition(TargetID targetId) const {
    auto target = GetTarget(targetId);
    if (target.has_value()) {
        return std::visit([](auto&& obj) -> Vector3 {
            return obj->GetWorldPosition();
        },
            target.value());
    }
    return Vector3::ZeroVector();
}

bool TargetManager::IsTargetValid(TargetID targetId) const {
    return targetId != INVALID_TARGET_ID && targets_.find(targetId) != targets_.end();
}