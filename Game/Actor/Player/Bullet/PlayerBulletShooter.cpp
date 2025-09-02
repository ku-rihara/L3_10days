#include "PlayerBulletShooter.h"
#include "Actor/Player/Player.h"
#include "BulletFactory.h"
#include "Frame/Frame.h"
#include "input/Input.h"
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

    // 発射状態の初期化
    for (auto& state : shooterStates_) {
        state = ShooterState{};
    }

    // 初期弾数設定
    InitializeAmmo();

    currentBulletType_ = BulletType::NORMAL;
}

void PlayerBulletShooter::InitializeAmmo() {
    // 各弾種の初期弾数を設定
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

    // 発射処理
    UpdateShooting(player);

    // 弾丸の更新
    UpdateBullets();

    // リロード処理
    UpdateReload();

    // 非アクティブな弾丸を削除
    CleanupInactiveBullets();
}

void PlayerBulletShooter::HandleInput() {
    Input* input = Input::GetInstance();

    // ToDo::ボタン配置決める

    // 弾種切り替え（Tabキー）
    if (input->TrrigerKey(DIK_TAB)) {
        SwitchBulletType();
    }

    // 手動リロード（Rキー）
    if (input->TrrigerKey(DIK_R)) {
        StartReload(currentBulletType_);
    }

    // 発射ボタン
    bool shootInput = input->PushKey(DIK_J) || Input::IsPressPad(0, XINPUT_GAMEPAD_A);

    size_t typeIndex    = static_cast<size_t>(currentBulletType_);
    ShooterState& state = shooterStates_[typeIndex];

    if (shootInput && CanShoot(currentBulletType_)) {
        state.isShooting = true;
    } else {
        state.isShooting = false;
    }
}

void PlayerBulletShooter::UpdateShooting(const Player* player) {
    float deltaTime     = Frame::DeltaTime();
    size_t typeIndex    = static_cast<size_t>(currentBulletType_);
    ShooterState& state = shooterStates_[typeIndex];

    // 発射間隔の更新
    if (state.intervalTimer > 0.0f) {
        state.intervalTimer -= deltaTime;
    }

    // 発射処理
    if (state.isShooting && CanShoot(currentBulletType_) && state.intervalTimer <= 0.0f) {
        FireBullets(player, currentBulletType_);

        // 発射間隔をリセット
        state.intervalTimer = shooterParameters_[typeIndex].intervalTime;

        // 弾数を減らす
        int32_t shotNum   = shooterParameters_[typeIndex].shotNum;
        state.currentAmmo = std::max(0, state.currentAmmo - shotNum);

        // 弾切れチェック
        if (state.currentAmmo <= 0) {
            StartReload(currentBulletType_);
        }
    }
}

void PlayerBulletShooter::FireBullets(const Player* player, BulletType type) {
    size_t typeIndex = static_cast<size_t>(type);
    int32_t shotNum  = shooterParameters_[typeIndex].shotNum;

    Vector3 playerPos    = player->GetPosition();
    Vector3 forwardDir   = player->GetForwardVector();
    Quaternion playerRot = player->GetQuaternion();

    for (int32_t i = 0; i < shotNum; ++i) {
        // 新しい弾丸を動的に生成
        auto bullet = BulletFactory::CreateBullet(type);
        if (bullet) {
            bullet->Init();
            bullet->SetParameter(type, bulletParameters_[typeIndex]);

            // 発射位置を少し前方にオフセット
            Vector3 firePos = playerPos + forwardDir * 2.0f;

            // 複数発射の場合は少し散らす
            Vector3 fireDir = forwardDir;
            if (shotNum > 1) {
                float spreadAngle = 0.1f;
                float angleOffset = (i - (shotNum - 1) * 0.5f) * spreadAngle;

                // Y軸周りに回転させて散らす
                Quaternion spreadRot = Quaternion::MakeRotateAxisAngle(Vector3::ToUp(), angleOffset);
                fireDir              = spreadRot.RotateVector(forwardDir);
            }

            bullet->Fire(firePos, fireDir, playerRot);

            // アクティブな弾丸リストに追加
            activeBullets_.push_back(std::move(bullet));
        }
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

        if (state.isReloading) {
            state.reloadTimer -= deltaTime;

            if (state.reloadTimer <= 0.0f) {
                // リロード完了
                state.isReloading = false;
                state.currentAmmo = shooterParameters_[i].maxBulletNum;
                state.reloadTimer = 0.0f;
            }
        }
    }
}

bool PlayerBulletShooter::CanShoot(BulletType type) const {
    size_t typeIndex          = static_cast<size_t>(type);
    const ShooterState& state = shooterStates_[typeIndex];

    return !state.isReloading && state.currentAmmo > 0 && state.intervalTimer <= 0.0f;
}

void PlayerBulletShooter::StartReload(BulletType type) {
    size_t typeIndex    = static_cast<size_t>(type);
    ShooterState& state = shooterStates_[typeIndex];

    if (!state.isReloading && state.currentAmmo < shooterParameters_[typeIndex].maxBulletNum) {
        state.isReloading = true;
        state.reloadTimer = shooterParameters_[typeIndex].reloadTime;
    }
}

void PlayerBulletShooter::SwitchBulletType() {
    int32_t nextType   = (static_cast<int32_t>(currentBulletType_) + 1) % static_cast<int32_t>(BulletType::COUNT);
    currentBulletType_ = static_cast<BulletType>(nextType);
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

BulletType PlayerBulletShooter::GetCurrentBulletType() const {
    return currentBulletType_;
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

void PlayerBulletShooter::BindParams() {
    for (uint32_t i = 0; i < bulletParameters_.size(); ++i) {
        // 弾のパラメータ
        globalParameter_->Bind(groupName_, "lifeTime" + std::to_string(int(i + 1)), &bulletParameters_[i].lifeTime);
        globalParameter_->Bind(groupName_, "speed" + std::to_string(int(i + 1)), &bulletParameters_[i].speed);
        // 発射パラメータ
        globalParameter_->Bind(groupName_, "intervalTime" + std::to_string(int(i + 1)), &shooterParameters_[i].intervalTime);
        globalParameter_->Bind(groupName_, "maxBulletNum" + std::to_string(int(i + 1)), &shooterParameters_[i].maxBulletNum);
        globalParameter_->Bind(groupName_, "shotNum" + std::to_string(int(i + 1)), &shooterParameters_[i].shotNum);
        globalParameter_->Bind(groupName_, "reloadTime" + std::to_string(int(i + 1)), &shooterParameters_[i].reloadTime);
    }
}

void PlayerBulletShooter::DrawEnemyParamUI(BulletType type) {
    // 弾のパラメータ
    ImGui::SeparatorText("BulletParameter");
    ImGui::DragFloat("LifeTime", &bulletParameters_[static_cast<size_t>(type)].lifeTime, 0.01f);
    ImGui::DragFloat("Speed", &bulletParameters_[static_cast<size_t>(type)].speed, 0.01f);

    // 発射のパラメータ
    ImGui::SeparatorText("ShooterParameter");
    ImGui::DragFloat("intervalTime", &shooterParameters_[static_cast<size_t>(type)].intervalTime, 0.01f);
    ImGui::DragFloat("ReloadTime", &shooterParameters_[static_cast<size_t>(type)].reloadTime, 0.01f);
    ImGui::InputInt("maxBulletNum", &shooterParameters_[static_cast<size_t>(type)].maxBulletNum);
    ImGui::InputInt("shotNum", &shooterParameters_[static_cast<size_t>(type)].shotNum);
}

void PlayerBulletShooter::AdjustParam() {
#ifdef _DEBUG

    if (ImGui::CollapsingHeader(groupName_.c_str())) {
        ImGui::PushID(groupName_.c_str());

        // 現在の弾種表示
        ImGui::SeparatorText("Current Status");
        ImGui::Text("Current Bullet Type: %s", typeNames_[static_cast<size_t>(currentBulletType_)].c_str());

        size_t currentTypeIndex          = static_cast<size_t>(currentBulletType_);
        const ShooterState& currentState = shooterStates_[currentTypeIndex];

        ImGui::Text("Ammo: %d / %d", currentState.currentAmmo, shooterParameters_[currentTypeIndex].maxBulletNum);
        ImGui::Text("Active Bullets: %d", GetActiveBulletCount());
        ImGui::Text("Reloading: %s", currentState.isReloading ? "Yes" : "No");
        if (currentState.isReloading) {
            ImGui::Text("Reload Progress: %.1f%%", GetReloadProgress(currentBulletType_) * 100.0f);
        }


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