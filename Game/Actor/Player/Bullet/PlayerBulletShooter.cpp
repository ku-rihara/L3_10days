#include "PlayerBulletShooter.h"
#include <imgui.h>

void PlayerBulletShooter::Init() {

    /// グローバルパラメータ
    globalParameter_ = GlobalParameter::GetInstance();
    globalParameter_->CreateGroup(groupName_, false);
    BindParams();
    globalParameter_->SyncParamForGroup(groupName_);

    // 名前セット
    typeNames_[static_cast<int32_t>(BulletType::NORMAL)] = "Normal";
    typeNames_[static_cast<int32_t>(BulletType::MISSILE)] = "Missile";
}

void PlayerBulletShooter::BindParams() {

    for (uint32_t i = 0; i < bulletParameters_.size(); ++i) {
        // 弾のパラメータ
        globalParameter_->Bind(groupName_, "isHoming" + std::to_string(int(i + 1)), &bulletParameters_[i].isHoming);
        globalParameter_->Bind(groupName_, "lifeTime" + std::to_string(int(i + 1)), &bulletParameters_[i].lifeTime);
        globalParameter_->Bind(groupName_, "speed" + std::to_string(int(i + 1)), &bulletParameters_[i].speed);
        // 発射パラメータ
        globalParameter_->Bind(groupName_, "intervalTime" + std::to_string(int(i + 1)), &shooterParameters_[i].intervalTime);
        globalParameter_->Bind(groupName_, "maxBulletNum" + std::to_string(int(i + 1)), &shooterParameters_[i].maxBulletNum);
        globalParameter_->Bind(groupName_, "reloadTime" + std::to_string(int(i + 1)), &shooterParameters_[i].reloadTime);
    }
}

void PlayerBulletShooter::AdjustParam() {
#ifdef _DEBUG

    if (ImGui::CollapsingHeader(groupName_.c_str())) {
        ImGui::PushID(groupName_.c_str());

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

void PlayerBulletShooter::DrawEnemyParamUI(BulletType type) {

    // 弾のパラメータ
    ImGui::SeparatorText("BulletParameter");
    ImGui::Checkbox("IsHoming", &bulletParameters_[static_cast<size_t>(type)].isHoming);
    ImGui::DragFloat("LifeTime", &bulletParameters_[static_cast<size_t>(type)].lifeTime, 0.01f);
    ImGui::DragFloat("Speed", &bulletParameters_[static_cast<size_t>(type)].speed, 0.01f);

    // 発射のパラメータ
    ImGui::SeparatorText("ShooterParameter");
    ImGui::DragFloat("intervalTime", &shooterParameters_[static_cast<size_t>(type)].intervalTime, 0.01f);
    ImGui::DragFloat("ReloadTime", &shooterParameters_[static_cast<size_t>(type)].reloadTime, 0.01f);
    ImGui::InputInt("maxBulletNum", &shooterParameters_[static_cast<size_t>(type)].maxBulletNum);
}