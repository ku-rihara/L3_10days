#include "PlayerReticle.h"
// obj
#include "3d/ViewProjection.h"
#include "Actor/Player/Player.h"
// math
#include "Matrix4x4.h"
// base
#include "base/TextureManager.h"

#include <imgui.h>

void PlayerReticle::Init() {

    // グローバルパラメータ
    globalParameter_ = GlobalParameter::GetInstance();
    globalParameter_->CreateGroup(groupName_, false);
    BindParams();
    globalParameter_->SyncParamForGroup(groupName_);

    uint32_t textureIndex = TextureManager::GetInstance()->LoadTexture("Resources/texture/UI/Reticle.png");
    sprite_.reset(Sprite::Create(textureIndex, {0, 0}, {1, 1, 1, 1}));
    sprite_->anchorPoint_ = Vector2(0.5f, 0.5f);
}

void PlayerReticle::Update(const Player* player, const ViewProjection* viewProjection) {
    if (!player || !viewProjection) {
        return;
    }

    // プレイヤーの座標と前方ベクトル
    Vector3 playerPos  = player->GetWorldPosition();
    Vector3 forwardDir = player->GetForwardVector();

    // レティクル位置
    worldPos_ = playerPos + (forwardDir * forwardDistance_);

    // 2D座標に変換
    Vector3 positionScreen = ScreenTransform(worldPos_, *viewProjection);
    Vector2 screenPos      = Vector2(positionScreen.x, positionScreen.y);
    screenPos_             = screenPos;

    //スプライトに適応
    sprite_->SetPosition(screenPos_);
}

void PlayerReticle::Draw() {
    if (sprite_) {
        sprite_->Draw();
    }
}

///=========================================================
/// バインド
///==========================================================
void PlayerReticle::BindParams() {
    globalParameter_->Bind(groupName_, "forwardDistance", &forwardDistance_);
}

///=========================================================
/// パラメータ調整
///==========================================================
void PlayerReticle::AdjustParam() {

#ifdef _DEBUG
    if (ImGui::CollapsingHeader(groupName_.c_str())) {
        ImGui::PushID(groupName_.c_str());

        ImGui::DragFloat("forwardDistance", &forwardDistance_, 0.01f);
       

        // セーブ・ロード
        globalParameter_->ParamSaveForImGui(groupName_);
        globalParameter_->ParamLoadForImGui(groupName_);

        ImGui::PopID();
    }
#endif // DEBUG
}