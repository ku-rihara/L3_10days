#include "PlayerBulletShooter.h"
#include "Actor/Player/Player.h"
#include "BulletFactory.h"
#include "Frame/Frame.h"
#include "input/Input.h"
#include "audio/Audio.h"
#include "PlayerMissile.h"
#include "Actor/Player/LockOn/LockOn.h"
#undef max
#include <algorithm>
#include <imgui.h>


void PlayerBulletShooter::Init() {
    /// グローバルパラメータ
    globalParameter_ = GlobalParameter::GetInstance();
    globalParameter_->CreateGroup(groupName_, false);
    BindParams();
    globalParameter_->SyncParamForGroup(groupName_);

    // 名前セット
    typeNames_[static_cast<int32_t>(BulletType::NORMAL)]  = "Normal";
    typeNames_[static_cast<int32_t>(BulletType::MISSILE)] = "Missile";

    // 初期弾数設定
    InitializeAmmo();

     targetManager_ = TargetManager::GetInstance();
}

void PlayerBulletShooter::InitializeAmmo() {
    // 弾数Maxで初期化
    for (size_t i = 0; i < shooterStates_.size() && i < shooterParameters_.size(); ++i) {
        shooterStates_[i].currentAmmo = shooterParameters_[i].maxBulletNum;
    }
}

void PlayerBulletShooter::Update(const Player* player) {
    if (!player) {
        return;
    }

    // 入力処理
    HandleInput();

    // 各弾種の発射処理を独立して実行
    UpdateNormalBulletShooting(player);
    UpdateMissileShooting(player);

    // 弾丸の更新
    UpdateBullets();

    // リロード処理
    UpdateReload();

    // 非アクティブな弾丸を削除
    CleanupInactiveBullets();

    // ホーミングミサイルの状態をLockOn
    UpdateHomingMissileStatus();
}

void PlayerBulletShooter::HandleInput() {
    Input* input = Input::GetInstance();

    // 通常弾発射
    normalBulletInput_ = input->PushKey(DIK_J) || Input::IsPressPad(0, XINPUT_GAMEPAD_A) || input->IsPressMouse(0);

    // ミサイル発射
    missileInput_ = input->TrrigerKey(DIK_K) || Input::IsTriggerPad(0, XINPUT_GAMEPAD_B) || input->IsPressMouse(1);

    // 手動リロード
    if (input->TrrigerKey(DIK_R)) {
        // 両方の弾種をリロード
        StartReload(BulletType::NORMAL);
        StartReload(BulletType::MISSILE);
    }
}

void PlayerBulletShooter::UpdateNormalBulletShooting(const Player* player) {
    float deltaTime     = Frame::DeltaTime();
    size_t typeIndex    = static_cast<size_t>(BulletType::NORMAL);
    ShooterState& state = shooterStates_[typeIndex];

    // 発射間隔の更新
    if (state.intervalTimer > 0.0f) {
        state.intervalTimer -= deltaTime;
    }

    // 通常弾の発射処理
    if (normalBulletInput_ && CanShoot(BulletType::NORMAL) && state.intervalTimer <= 0.0f) {
        FireBullets(player, BulletType::NORMAL);
		// 発射SE再生
		int handle = Audio::GetInstance()->LoadWave("./resources/Sound/SE/BulletFire.wav");
		Audio::GetInstance()->PlayWave(handle, 0.05f, 0.5f);

        // 発射間隔をリセット
        state.intervalTimer = shooterParameters_[typeIndex].intervalTime;

        // 弾数を減らす
        state.currentAmmo = std::max(0, state.currentAmmo - 1);

        // 弾切れチェック
        if (state.currentAmmo <= 0) {
            StartReload(BulletType::NORMAL);
        }
    }
}

void PlayerBulletShooter::UpdateMissileShooting(const Player* player) {
    float deltaTime     = Frame::DeltaTime();
    size_t typeIndex    = static_cast<size_t>(BulletType::MISSILE);
    ShooterState& state = shooterStates_[typeIndex];

    // 発射間隔の更新
    if (state.intervalTimer > 0.0f) {
        state.intervalTimer -= deltaTime;
    }

    // 通常弾の発射処理
    if (missileInput_ && CanShoot(BulletType::MISSILE) && state.intervalTimer <= 0.0f) {
        FireBullets(player, BulletType::MISSILE);

		// 発射SE再生
		int handle = Audio::GetInstance()->LoadWave("./resources/Sound/SE/MissileFire.wav");
		Audio::GetInstance()->PlayWave(handle, 0.1f);

        // 発射間隔をリセット
        state.intervalTimer = shooterParameters_[typeIndex].intervalTime;

        // 弾数を減らす
        state.currentAmmo = std::max(0, state.currentAmmo - 1);

        // 弾切れチェック
        if (state.currentAmmo <= 0) {
            StartReload(BulletType::MISSILE);
        }
    }
}

 void PlayerBulletShooter::FireBullets(const Player* player, BulletType type) {
    size_t typeIndex = static_cast<size_t>(type);

    // 新しい弾丸を生成
    auto bullet = BulletFactory::CreateBullet(type);
    if (bullet) {
        bullet->Init();
        bullet->SetParameter(type, bulletParameters_[typeIndex]);

        // ミサイルの場合は専用パラメータを設定
        if (type == BulletType::MISSILE) {
            auto* missile = dynamic_cast<PlayerMissile*>(bullet.get());
            if (missile) {
                missile->SetMissileParameters(typeSpecificParams_.missile);

                // ターゲットが存在する場合、TargetManagerに登録してIDを取得
                const LockOn::LockOnVariant* currentTarget = pLockOn_->GetCurrentTarget();
                if (currentTarget && targetManager_) {
                    TargetID targetId = targetManager_->RegisterTarget(*currentTarget);
                    missile->SetTargetID(targetId);
                }
            }
        }

        // 発射
        bullet->Fire(*player, pLockOn_->GetCurrentTarget());

        // 弾丸リストに追加
        activeBullets_.push_back(std::move(bullet));
    }
}


void PlayerBulletShooter::UpdateBullets() {
    for (auto& bullet : activeBullets_) {
        if (bullet && bullet->GetIsActive()) {
            bullet->Update();
        }
    }
}

void PlayerBulletShooter::CleanupInactiveBullets() {
    // 非アクティブな弾丸を削除
    activeBullets_.erase(
        std::remove_if(activeBullets_.begin(), activeBullets_.end(),
            [](const std::unique_ptr<BasePlayerBullet>& bullet) {
                return !bullet || !bullet->GetIsActive();
            }),
        activeBullets_.end());
}

void PlayerBulletShooter::UpdateReload() {
    float deltaTime = Frame::DeltaTime();

    for (size_t i = 0; i < shooterStates_.size(); ++i) {
        ShooterState& state = shooterStates_[i];

        // 　リロード中の処理
        if (!state.isReloading) {
            continue;
        }
        // リロードタイマー更新
        state.reloadTimer -= deltaTime;

        // リロード完了
        if (state.reloadTimer > 0.0f) {
            continue;
        }

        state.isReloading = false;
        state.currentAmmo = shooterParameters_[i].maxBulletNum;
        state.reloadTimer = 0.0f;
    }
}

bool PlayerBulletShooter::CanShoot(BulletType type) const {
    size_t typeIndex          = static_cast<size_t>(type);
    const ShooterState& state = shooterStates_[typeIndex];
    return !state.isReloading && state.currentAmmo > 0 && state.intervalTimer <= 0.0f;
}

bool PlayerBulletShooter::CanShootMissile() const {
    const ShooterState& state = shooterStates_[static_cast<size_t>(BulletType::MISSILE)];
    return !state.isReloading && state.currentAmmo > 0 && !missileBurstState_.isBursting && state.intervalTimer <= 0.0f;
}

void PlayerBulletShooter::StartReload(BulletType type) {
    size_t typeIndex    = static_cast<size_t>(type);
    ShooterState& state = shooterStates_[typeIndex];

    if (!state.isReloading && state.currentAmmo < shooterParameters_[typeIndex].maxBulletNum) {
        state.isReloading = true;
        state.reloadTimer = shooterParameters_[typeIndex].reloadTime;

        // ミサイルの連射状態リセット
        if (type == BulletType::MISSILE) {
            missileBurstState_.isBursting        = false;
            missileBurstState_.currentBurstCount = 0;
        }
    }
}

void PlayerBulletShooter::UpdateHomingMissileStatus() {
    if (!pLockOn_) {
        return;
    }

    // アクティブなホーミングミサイルがあるかチェック
    bool hasActiveHomingMissile = false;

    for (const auto& bullet : activeBullets_) {
        if (bullet && bullet->GetIsActive()) {
            // ミサイルタイプかつアクティブな場合
            PlayerMissile* missile = dynamic_cast<PlayerMissile*>(bullet.get());
            if (missile) {
                hasActiveHomingMissile = true;
                break;
            }
        }
    }

    // LockOnシステムに状態を通知
    pLockOn_->SetHomingMissileActive(hasActiveHomingMissile);
}


void PlayerBulletShooter::BindParams() {
    for (uint32_t i = 0; i < bulletParameters_.size(); ++i) {
        // 弾のパラメータ
        globalParameter_->Bind(groupName_, "lifeTime" + std::to_string(int(i + 1)), &bulletParameters_[i].lifeTime);
        globalParameter_->Bind(groupName_, "speed" + std::to_string(int(i + 1)), &bulletParameters_[i].speed);
        globalParameter_->Bind(groupName_, "damage" + std::to_string(int(i + 1)), &bulletParameters_[i].damage);
        globalParameter_->Bind(groupName_, "breakRadius" + std::to_string(int(i + 1)), &bulletParameters_[i].breakRadius);
        // 発射パラメータ
        globalParameter_->Bind(groupName_, "intervalTime" + std::to_string(int(i + 1)), &shooterParameters_[i].intervalTime);
        globalParameter_->Bind(groupName_, "maxBulletNum" + std::to_string(int(i + 1)), &shooterParameters_[i].maxBulletNum);
        globalParameter_->Bind(groupName_, "reloadTime" + std::to_string(int(i + 1)), &shooterParameters_[i].reloadTime);
    }
    // ミサイル専用パラメータ
    globalParameter_->Bind(groupName_, "missileTrackingStrength", &typeSpecificParams_.missile.trackingStrength);
    globalParameter_->Bind(groupName_, "missileMaxTurnRate", &typeSpecificParams_.missile.maxTurnRate);
}

void PlayerBulletShooter::DrawEnemyParamUI(BulletType type) {
    // 弾のパラメータ
    ImGui::SeparatorText("BulletParameter");
    ImGui::DragFloat("LifeTime", &bulletParameters_[static_cast<size_t>(type)].lifeTime, 0.01f);
    ImGui::DragFloat("Speed", &bulletParameters_[static_cast<size_t>(type)].speed, 0.01f);
    ImGui::DragFloat("Damage", &bulletParameters_[static_cast<size_t>(type)].damage, 0.01f);
    ImGui::DragFloat("BreakRadius", &bulletParameters_[static_cast<size_t>(type)].breakRadius, 0.01f);


    // 発射のパラメータ
    ImGui::SeparatorText("ShooterParameter");
    ImGui::DragFloat("intervalTime", &shooterParameters_[static_cast<size_t>(type)].intervalTime, 0.01f);
    ImGui::DragFloat("ReloadTime", &shooterParameters_[static_cast<size_t>(type)].reloadTime, 0.01f);
    ImGui::InputInt("maxBulletNum", &shooterParameters_[static_cast<size_t>(type)].maxBulletNum);

    // ミサイル専用パラメータ
    if (type == BulletType::MISSILE) {
        ImGui::SeparatorText("MissileParameter");
        ImGui::DragFloat("TrackingStrength", &typeSpecificParams_.missile.trackingStrength, 0.01f);
        ImGui::DragFloat("MaxTurnRate", &typeSpecificParams_.missile.maxTurnRate, 0.01f);
    }
}

void PlayerBulletShooter::AdjustParam() {
#ifdef _DEBUG

    if (ImGui::CollapsingHeader(groupName_.c_str())) {
        ImGui::PushID(groupName_.c_str());

        // 現在の状態表示
        ImGui::SeparatorText("Current Status");

        // パラメータ編集
        for (size_t i = 0; i < static_cast<size_t>(BulletType::COUNT); ++i) {
            BulletType type = static_cast<BulletType>(i);
            ImGui::SeparatorText(typeNames_[i].c_str());
            ImGui::PushID(typeNames_[i].c_str());

            DrawEnemyParamUI(type);

            ImGui::PopID();
        }

        globalParameter_->ParamSaveForImGui(groupName_);
        globalParameter_->ParamLoadForImGui(groupName_);

        ImGui::PopID();
    }

#endif
}

int32_t PlayerBulletShooter::GetCurrentAmmo(BulletType type) const {
    size_t typeIndex = static_cast<size_t>(type);
    return shooterStates_[typeIndex].currentAmmo;
}

bool PlayerBulletShooter::IsReloading(BulletType type) const {
    size_t typeIndex = static_cast<size_t>(type);
    return shooterStates_[typeIndex].isReloading;
}

float PlayerBulletShooter::GetReloadProgress(BulletType type) const {
    size_t typeIndex          = static_cast<size_t>(type);
    const ShooterState& state = shooterStates_[typeIndex];

    if (!state.isReloading) {
        return 1.0f;
    }

    float totalReloadTime = shooterParameters_[typeIndex].reloadTime;
    return 1.0f - (state.reloadTimer / totalReloadTime);
}

std::vector<BasePlayerBullet*> PlayerBulletShooter::GetActiveBullets() const {
    std::vector<BasePlayerBullet*> bullets;
    bullets.reserve(activeBullets_.size());

    for (const auto& bullet : activeBullets_) {
        if (bullet && bullet->GetIsActive()) {
            bullets.push_back(bullet.get());
        }
    }

    return bullets;
}

int32_t PlayerBulletShooter::GetActiveBulletCount() const {
    int32_t count = 0;
    for (const auto& bullet : activeBullets_) {
        if (bullet && bullet->GetIsActive()) {
            ++count;
        }
    }
    return count;
}

void PlayerBulletShooter::ClearAllBullets() {
    activeBullets_.clear();
}
