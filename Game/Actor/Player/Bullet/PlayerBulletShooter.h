#pragma once
#include "Actor/Player/TargetManager/TargetManager.h "
#include "BasePlayerBullet.h"
#include "MissileSlotManager.h" // 追加
#include "utility/ParameterEditor/GlobalParameter.h"
#include <array>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

class LockOn;
class BasePlayerBullet;
class Player;
class ViewProjection;

struct ShooterParameter {
    float intervalTime;
    int32_t maxBulletNum;
    float reloadTime;
};

struct ShooterState {
    bool isShooting;
    bool isReloading;
    int32_t currentAmmo;
    float intervalTimer;
    float reloadTimer;
};

// ミサイル独自パラメータ
struct MissileParameter {
    float trackingStrength;
    float maxTurnRate;
    float maxSpeed;
    float acceleration;
};

// ミサイル設定パラメータ
struct MissileSystemParameter {
    int32_t maxSlots    = 2; // 最大スロット数
    float cooldownTime  = 3.0f; // 各スロットのクールダウン時間
    float shootInterval = 0.1f; // 発射間隔（連続発射防止）
};

struct TypeSpecificParameters {
    MissileParameter missile;
    MissileSystemParameter missileSystem;
};

class PlayerBulletShooter {
public:
    PlayerBulletShooter()  = default;
    ~PlayerBulletShooter() = default;

public:
    void Init();
    void Update(const Player* player);

    ///-------------------------------------------------------------------------------------
    /// Editor
    ///-------------------------------------------------------------------------------------
    void BindParams();
    void AdjustParam();
    void DrawEnemyParamUI(BulletType type);

private:
    // 初期化関数
    void InitializeAmmo();

    // 入力処理
    void HandleInput();
    void UpdateHomingMissileStatus();

    // 発射処理
    void UpdateNormalBulletShooting(const Player* player);
    void UpdateMissileShooting(const Player* player);

    void FireBullets(const Player* player, BulletType type);
    void FireMissile(const Player* player); // ミサイル専用発射メソッド

    // 弾丸更新・管理
    void UpdateBullets();
    void CleanupInactiveBullets();

    // リロード処理
    void UpdateReload();
    void StartReload(BulletType type);
    bool CanShoot(BulletType type) const;
    bool CanShootMissile() const;

    void ClearAllBullets();

private:
    TargetManager* targetManager_ = nullptr;

private:
    // globalParameter
    GlobalParameter* globalParameter_;
    const std::string groupName_ = "Bullets";
    LockOn* pLockOn_;
    const ViewProjection* viewProjection_ = nullptr;

    // パラメータ
    std::array<BulletParameter, static_cast<int32_t>(BulletType::COUNT)> bulletParameters_;
    std::array<ShooterParameter, static_cast<int32_t>(BulletType::COUNT)> shooterParameters_;
    std::array<std::string, static_cast<int32_t>(BulletType::COUNT)> typeNames_;

    // 弾種別専用パラメータ
    TypeSpecificParameters typeSpecificParams_;

    // アクティブな弾丸のリスト
    std::vector<std::unique_ptr<BasePlayerBullet>> activeBullets_;

    // 発射状態
    std::array<ShooterState, static_cast<int32_t>(BulletType::COUNT)> shooterStates_;

    // ミサイルスロット管理
    MissileSlotManager missileSlotManager_;
    float missileShootTimer_ = 0.0f; 

    // 入力状態
    bool normalBulletInput_ = false;
    bool missileInput_      = false;

public:
    /// -----------------------------------------------------------------
    /// Getter
    /// -----------------------------------------------------------------
    std::vector<BasePlayerBullet*> GetActiveBullets() const;
    int32_t GetCurrentAmmo(BulletType type) const;
    bool IsReloading(BulletType type) const;
    float GetReloadProgress(BulletType type) const;
    int32_t GetActiveBulletCount() const;
    bool CanFireMissileFromSlot(int32_t slotIndex) const { return missileSlotManager_.CanFireFromSlot(slotIndex); }
    float GetMissileSlotRemainingCooldown(int32_t slotIndex) const { return missileSlotManager_.GetSlotRemainingCooldown(slotIndex); }

    const MissileParameter& GetMissileParameter() const { return typeSpecificParams_.missile; }

    // ミサイルスロット関連のGetter
    int32_t GetAvailableMissileSlots() const { return missileSlotManager_.GetAvailableSlotCount(); }
    int32_t GetMaxMissileSlots() const { return missileSlotManager_.GetMaxSlots(); }
    bool IsAnyMissileSlotAvailable() const { return missileSlotManager_.HasAnyAvailableSlot(); }
    float GetMissileSlotCooldownProgress(int32_t slotIndex) const { return missileSlotManager_.GetSlotCooldownProgress(slotIndex); }
    bool IsMissileSlotAvailable(int32_t slotIndex) const { return missileSlotManager_.IsSlotAvailable(slotIndex); }
    const float& GetMissileCollTimeMax() const { return typeSpecificParams_.missileSystem.cooldownTime; }

    /// -----------------------------------------------------------------
    /// Setter
    /// -----------------------------------------------------------------
    void SetViewProjection(const ViewProjection* vp) { viewProjection_ = vp; }
    void SetLockOn(LockOn* lockOn) { pLockOn_ = lockOn; }
};