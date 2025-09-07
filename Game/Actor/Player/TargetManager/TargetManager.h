#pragma once
#include "Actor/Player/LockOn/LockOn.h"
#include "Vector3.h"
#include <cstdint>
#include <optional>
#include <unordered_map>

using TargetID                       = uint32_t;
constexpr TargetID INVALID_TARGET_ID = 0;

class TargetManager {
public:
    static TargetManager* GetInstance();

    // ターゲット登録・削除
    TargetID RegisterTarget(const LockOn::LockOnVariant& target);
    void UnregisterTarget(TargetID targetId);
    void UnregisterTarget(const LockOn::LockOnVariant& target);

    // ターゲット情報取得
    std::optional<LockOn::LockOnVariant> GetTarget(TargetID targetId) const;
    Vector3 GetTargetPosition(TargetID targetId) const;
    bool IsTargetValid(TargetID targetId) const;

    // デバッグ
    size_t GetRegisteredTargetCount() const { return targets_.size(); }
    void ClearAllTargets() {
        targets_.clear();
        nextId_ = 1;
    }

private:
    TargetManager()  = default;
    ~TargetManager() = default;

    std::unordered_map<TargetID, LockOn::LockOnVariant> targets_;
    TargetID nextId_ = 1; // 0は無効IDとして予約

    // シングルトン
    TargetManager(const TargetManager&)            = delete;
    TargetManager& operator=(const TargetManager&) = delete;
};