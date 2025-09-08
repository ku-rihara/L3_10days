#pragma once
#include <array>
#include <cstdint>

// ミサイルスロットの状態
struct MissileSlotState {
    bool isAvailable    = true; // 使用可能かどうか
    float cooldownTimer = 0.0f; // クールダウンタイマー
    bool isReloading    = false; // リロード中かどうか
};

// ミサイルスロット管理クラス
class MissileSlotManager {
public:
    static constexpr int32_t MAX_MISSILE_SLOTS = 4; // 最大ミサイルスロット数

public:
    MissileSlotManager()  = default;
    ~MissileSlotManager() = default;

    // 初期化
    void Initialize(int32_t maxSlots, float cooldownTime);

    // 更新
    void Update(float deltaTime);

    // 発射可能なスロットを取得
    int32_t GetAvailableSlot() const;

    // ミサイルを発射（指定スロットのクールダウン開始）
    bool FireMissile(int32_t slotIndex);

    // 手動リロード開始
    void StartManualReload();

    // ゲッター
    bool IsSlotAvailable(int32_t slotIndex) const;
    float GetSlotCooldownProgress(int32_t slotIndex) const;
    int32_t GetAvailableSlotCount() const;
    bool HasAnyAvailableSlot() const;

    // パラメータ取得
    float GetCooldownTime() const { return cooldownTime_; }
    int32_t GetMaxSlots() const { return maxSlots_; }

private:
    std::array<MissileSlotState, MAX_MISSILE_SLOTS> slots_;
    int32_t maxSlots_   = 2; // 使用するスロット数
    float cooldownTime_ = 3.0f; // クールダウン時間
};