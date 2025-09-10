#include "MissileSlotManager.h"
#include <algorithm>

void MissileSlotManager::Initialize(int32_t maxSlots, float cooldownTime) {
    maxSlots_     = std::clamp(maxSlots, 1, maxSlots_);
    cooldownTime_ = cooldownTime;

    // 全スロットを利用可能状態で初期化
    for (int32_t i = 0; i < maxSlots_; ++i) {
        slots_[i].isAvailable   = true;
        slots_[i].cooldownTimer = 0.0f;
        slots_[i].isReloading   = false;
    }

    // 使用しないスロットは無効化
    for (int32_t i = maxSlots_; i < maxSlots_; ++i) {
        slots_[i].isAvailable   = false;
        slots_[i].cooldownTimer = 0.0f;
        slots_[i].isReloading   = false;
    }
}

void MissileSlotManager::Update(float deltaTime) {
    for (int32_t i = 0; i < maxSlots_; ++i) {
        auto& slot = slots_[i];

        if (slot.isReloading && slot.cooldownTimer > 0.0f) {
            slot.cooldownTimer -= deltaTime;

            // クールダウン完了
            if (slot.cooldownTimer <= 0.0f) {
                slot.isReloading   = false;
                slot.isAvailable   = true;
                slot.cooldownTimer = 0.0f;
            }
        }
    }
}

int32_t MissileSlotManager::GetAvailableSlot() const {
    for (int32_t i = 0; i < maxSlots_; ++i) {
        if (slots_[i].isAvailable && !slots_[i].isReloading) {
            return i;
        }
    }
    return -1;
}

bool MissileSlotManager::FireMissile(int32_t slotIndex) {
    if (slotIndex < 0 || slotIndex >= maxSlots_) {
        return false;
    }

    auto& slot = slots_[slotIndex];
    if (!slot.isAvailable || slot.isReloading) {
        return false;
    }

    // クールダウン開始
    slot.isAvailable   = false;
    slot.isReloading   = true;
    slot.cooldownTimer = cooldownTime_;

    return true;
}

void MissileSlotManager::StartManualReload() {
    for (int32_t i = 0; i < maxSlots_; ++i) {
        auto& slot = slots_[i];
        if (!slot.isAvailable && !slot.isReloading) {
            slot.isReloading   = true;
            slot.cooldownTimer = cooldownTime_;
        }
    }
}

bool MissileSlotManager::IsSlotAvailable(int32_t slotIndex) const {
    if (slotIndex < 0 || slotIndex >= maxSlots_) {
        return false;
    }
    return slots_[slotIndex].isAvailable && !slots_[slotIndex].isReloading;
}

float MissileSlotManager::GetSlotCooldownProgress(int32_t slotIndex) const {
    if (slotIndex < 0 || slotIndex >= maxSlots_) {
        return 0.0f;
    }

    const auto& slot = slots_[slotIndex];
    if (!slot.isReloading) {
        return slot.isAvailable ? 1.0f : 0.0f;
    }

    return 1.0f - (slot.cooldownTimer / cooldownTime_);
}

int32_t MissileSlotManager::GetAvailableSlotCount() const {
    int32_t count = 0;
    for (int32_t i = 0; i < maxSlots_; ++i) {
        if (slots_[i].isAvailable && !slots_[i].isReloading) {
            count++;
        }
    }
    return count;
}

bool MissileSlotManager::HasAnyAvailableSlot() const {
    return GetAvailableSlot() != -1;
}

bool MissileSlotManager::CanFireFromSlot(int32_t slotIndex) const {
    return IsSlotAvailable(slotIndex);
}

float MissileSlotManager::GetSlotRemainingCooldown(int32_t slotIndex) const {
    if (slotIndex < 0 || slotIndex >= maxSlots_) {
        return 0.0f;
    }

    const auto& slot = slots_[slotIndex];
    return slot.isReloading ? slot.cooldownTimer : 0.0f;
}