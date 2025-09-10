#include "PlayerStation.h"

#include "Actor/NPC/PlayerNPC.h"
#include "Actor/NPC/Pool/NpcPool.h"
#include "Frame/Frame.h"
#include"base/TextureManager.h"
#include "imgui.h"

PlayerStation::PlayerStation() { BaseStation::SetFaction(FactionType::Ally); }

PlayerStation::PlayerStation(const std::string& name) : BaseStation(name) {
    BaseStation::SetFaction(FactionType::Ally);
}

PlayerStation::~PlayerStation() { spawned_.clear(); }

/// ===================================================
/// 初期化
/// ===================================================
void PlayerStation::Init() {
    obj3d_.reset(Object3d::CreateModel("PlayerBase.obj"));
    BaseObject::Init();
    obj3d_->transform_.parent_ = &baseTransform_;

    globalParam_ = GlobalParameter::GetInstance();
    faction_     = FactionType::Ally;
    BaseStation::Init();

    int handle = TextureManager::GetInstance()->LoadTexture("Resources/Texture/UI/AllyReticle.png");
    bossReticle_.reset(Sprite::Create(handle, Vector2::ZeroVector(), Vector4::kWHITE()));
    bossReticle_->anchorPoint_ = Vector2(0.5f, 0.5f);
}

/// ===================================================
/// 更新
/// ===================================================
void PlayerStation::Update() {
    float dt = Frame::DeltaTime();
    // スポーン
    currentTime_ += dt;
    if (currentTime_ >= spawnInterbal_) {
        SpawnNPC(GetWorldPosition());
    }

    for (auto& enemy : spawned_) {
        enemy->Update();
    }

    BaseStation::Update();
}

/// ===================================================
/// npcのスポーン
/// ===================================================
void PlayerStation::SpawnNPC(const Vector3& spawnPos) {

    if (static_cast<int>(spawned_.size()) >= maxConcurrentUnits_)
        return;

    auto npc = pool_.Acquire();
    npc->Init();
    npc->SetFaction(FactionType::Ally);

    if (auto* rival = this->GetRivalStation()) {
        npc->SetTargetProvider(this);
    }

    npc->SetWorldPosition(spawnPos);
    npc->SetTarget(GetRivalStation());
    if (const auto* rc = GetRouteCollection()) {
        npc->AttachRoutes(rc);
    }
    spawned_.push_back(std::move(npc));
    currentTime_ = 0.0f;
}

void PlayerStation::SpriteUpdate(const ViewProjection& viewPro) {
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
void PlayerStation::DrawSprite() {
    if (!isDraw_) {
        return;
    }

    bossReticle_->Draw();
}