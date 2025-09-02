#pragma once
#include "BasePlayerBullet.h"
#include "utility/ParameterEditor/GlobalParameter.h"
#include <array>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

class BasePlayerBullet;
class Player;

struct ShooterParameter {
    float intervalTime;
    int32_t maxBulletNum;
    int32_t shotNum;
    float reloadTime;
};

struct ShooterState {
    bool isShooting     = false;
    bool isReloading    = false;
    int32_t currentAmmo = 0;
    float intervalTimer = 0.0f;
    float reloadTimer   = 0.0f;
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
    void UpdateShooting(const Player* player);
    void FireBullets(const Player* player, BulletType type);

    // 弾丸更新・管理
    void UpdateBullets();
    void CleanupInactiveBullets();

    // リロード処理
    void UpdateReload();
    void StartReload(BulletType type);
    bool CanShoot(BulletType type) const;

     void ClearAllBullets();

    // 弾種切り替え
    void SwitchBulletType();

private:
    // globalParameter
    GlobalParameter* globalParameter_;
    const std::string groupName_ = "Bullets";

    // パラメータ
    std::array<BulletParameter, static_cast<int32_t>(BulletType::COUNT)> bulletParameters_;
    std::array<ShooterParameter, static_cast<int32_t>(BulletType::COUNT)> shooterParameters_;
    std::array<std::string, static_cast<int32_t>(BulletType::COUNT)> typeNames_;

    // アクティブな弾丸のリスト
    std::vector<std::unique_ptr<BasePlayerBullet>> activeBullets_;

    // 発射状態
    std::array<ShooterState, static_cast<int32_t>(BulletType::COUNT)> shooterStates_;
    BulletType currentBulletType_ = BulletType::NORMAL;

public:
   
    /// -----------------------------------------------------------------
    /// Getter
    /// -----------------------------------------------------------------
    std::vector<BasePlayerBullet*> GetActiveBullets() const;
    int32_t GetCurrentAmmo(BulletType type) const;
    bool IsReloading(BulletType type) const;
    float GetReloadProgress(BulletType type) const;
    BulletType GetCurrentBulletType() const;
    int32_t GetActiveBulletCount() const;

    /// -----------------------------------------------------------------
    /// Setter
    /// -----------------------------------------------------------------
  
};