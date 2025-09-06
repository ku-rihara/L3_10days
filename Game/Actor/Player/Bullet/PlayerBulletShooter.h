#pragma once
#include "BasePlayerBullet.h"
#include "Actor/Player/LockOn/LockOn.h"
#include "utility/ParameterEditor/GlobalParameter.h"
#include <array>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

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
 
};

// ミサイル連射状態
struct MissileBurstState {
    int32_t currentBurstCount;
    float burstTimer;
    bool isBursting;
};

struct TypeSpecificParameters {
    MissileParameter missile;
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

    // 発射処理
    void UpdateNormalBulletShooting(const Player* player);
    void UpdateMissileShooting(const Player* player);

    void FireBullets(const Player* player, BulletType type);

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
    // globalParameter
    GlobalParameter* globalParameter_;
    const std::string groupName_ = "Bullets";
    std::unique_ptr<LockOn> lockOn_;
    const ViewProjection* viewProjection_ = nullptr;

    // パラメータ
    std::array<BulletParameter, static_cast<int32_t>(BulletType::COUNT)> bulletParameters_;
    std::array<ShooterParameter, static_cast<int32_t>(BulletType::COUNT)> shooterParameters_;
    std::array<std::string, static_cast<int32_t>(BulletType::COUNT)> typeNames_;

    // 弾種別専用パラメータ
    TypeSpecificParameters typeSpecificParams_;

    // アクティブな弾丸のリスト
    std::vector<std::unique_ptr<BasePlayerBullet>> activeBullets_;

    // 発射状態 - 各弾種独立
    std::array<ShooterState, static_cast<int32_t>(BulletType::COUNT)> shooterStates_;

    // ミサイル連射状態
    MissileBurstState missileBurstState_;

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

    const MissileParameter& GetMissileParameter() const { return typeSpecificParams_.missile; }

    int32_t GetMissileBurstCount() const { return missileBurstState_.currentBurstCount; }
    bool IsMissileBursting() const { return missileBurstState_.isBursting; }

    /// -----------------------------------------------------------------
    /// Setter
    /// -----------------------------------------------------------------
    void SetViewProjection(const ViewProjection* vp) { viewProjection_ = vp; }
};