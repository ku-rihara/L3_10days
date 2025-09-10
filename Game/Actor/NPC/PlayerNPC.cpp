#include "PlayerNPC.h"
#include "Actor/NPC/Bullet/FireController/NpcFierController.h"
#include"base/TextureManager.h"
#include <Frame/Frame.h>

PlayerNPC::PlayerNPC() {
    // 調整用グループ名
    groupName_ = "PlayerNPC";
    // 派閥
    SetFaction(FactionType::Ally);
}

PlayerNPC::~PlayerNPC() = default;

/// ===================================================
///  初期化
/// ===================================================
void PlayerNPC::Init() {
    if (isInitialized_) {
        return;
    }
    isInitialized_ = true;

    obj3d_.reset(Object3d::CreateModel("Enemy.obj")); // 味方npcだけどいったんモデルを代用
    BaseObject::Init();
    obj3d_->transform_.parent_ = &baseTransform_;

    int handle = TextureManager::GetInstance()->LoadTexture("Resources/Texture/UI/AllyReticle.png");
    bossReticle_.reset(Sprite::Create(handle, Vector2::ZeroVector(), Vector4::kWHITE()));
    bossReticle_->anchorPoint_ = Vector2(0.5f, 0.5f);

    NPC::Init();
    SetColor(Vector4(0.0f, 0.0f, 1.0f, 1.0f));
}

/// ===================================================
///  更新
/// ===================================================
void PlayerNPC::Update() {
    NPC::Update();
}

void PlayerNPC::SpriteUpdate(const ViewProjection& viewPro) {
    // 敵のワールド座標
    Vector3 positionWorld = GetWorldPosition();

    // ビュー空間に変換
    Vector3 positionView = TransformMatrix(positionWorld, viewPro.matView_);

    // デフォルト非表示
    isDraw_ = false;

    // ===== 後ろにいる場合は描画しない =====
    if (positionView.z <= 0.0f) {
        return;
    }

    // ===== スクリーン座標に変換 =====
    Vector3 positionScreen = ScreenTransform(positionWorld, viewPro);

    // 画面範囲チェック（例: 解像度 1280x720）
    if (positionScreen.x < 0.0f || positionScreen.x > 1280.0f || positionScreen.y < 0.0f || positionScreen.y > 720.0f) {
        return;
    }

    // ===== 表示可能 =====
    isDraw_ = true;

    bossReticle_->SetScale(Vector2(0.1f, 0.1f));
    bossReticle_->SetPosition(Vector2(positionScreen.x, positionScreen.y));   
}

void PlayerNPC::DrawSprite() {
    if (!isDraw_) {
        return;
    }

    bossReticle_->Draw();
}