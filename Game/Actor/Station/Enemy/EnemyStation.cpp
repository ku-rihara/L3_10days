#include "EnemyStation.h"

#include "Actor/NPC/EnemyNPC.h"
#include "Actor/NPC/Pool/NpcPool.h"
#include "Frame/Frame.h"
#include "Actor/Player/Player.h"
#include "utility/ParameterEditor/GlobalParameter.h"
#include "Actor/NPC/Bullet/FireController/NpcFierController.h"
#include "imgui.h"

EnemyStation::EnemyStation(){ BaseStation::SetFaction(FactionType::Enemy); }

EnemyStation::EnemyStation(const std::string& name) :
	BaseStation(name){ BaseStation::SetFaction(FactionType::Enemy); }

EnemyStation::~EnemyStation(){ spawned_.clear(); }

/// ===================================================
/// 初期化
/// ===================================================
void EnemyStation::Init(){
	obj3d_.reset(Object3d::CreateModel("EnemyBase.obj"));
	BaseObject::Init();
	obj3d_->transform_.parent_ = &baseTransform_;

	globalParam_ = GlobalParameter::GetInstance();
	faction_ = FactionType::Enemy;
	BaseStation::Init();

	fireController_ = std::make_unique<NpcFireController>();
	fireController_->Init();

	shootCooldown_ = shootInterval_;
}

/// ===================================================
/// 更新
/// ===================================================
void EnemyStation::Update(){
	float dt = Frame::DeltaTime();
	//スポーン
	currentTime_ += dt;
	if (currentTime_ >= spawnInterbal_){ SpawnNPC(GetWorldPosition()); }

	//弾の発射
	if (fireController_) fireController_->Tick();

	TryFire();

	for (auto& enemy : spawned_){ enemy->Update(); }

	BaseStation::Update();
}

/// ===================================================
/// npcのスポーン

void EnemyStation::SpawnNPC(const Vector3& pos){
	if (static_cast<int>(spawned_.size()) >= maxConcurrentUnits_) return;

	auto npc = pool_.Acquire();
	npc->Init();
	npc->SetFaction(FactionType::Enemy);

	if (auto* rival = this->GetRivalStation()){ npc->SetTargetProvider(this); }

	npc->SetWorldPosition(pos);
	npc->SetTarget(GetRivalStation());
	if (const auto* rc = GetRouteCollection()){ npc->AttachRoutes(rc); }
	spawned_.push_back(std::move(npc));
	currentTime_ = 0.0f;
}

void EnemyStation::SetPlayerPtr(const Player* player){ pPlayer_ = player; }

void EnemyStation::CollectTargets(std::vector<const BaseObject*>& out) const{
	BaseStation::CollectTargets(out);

	// 自分が Enemy で、プレイヤーが指定されていれば攻撃候補に追加
	if (GetFactionType() == FactionType::Enemy && pPlayer_){ out.push_back(static_cast<const BaseObject*>(pPlayer_)); }
}

void EnemyStation::TryFire() {
    if (!fireController_) return;
    if (!pPlayer_) return;

    const float dt = Frame::DeltaTime();

    // ===== バースト（直進連射）中の処理 =====
    if (burstActive_) {
        burstTimer_    -= dt;
        burstCooldown_ -= dt;

        if (burstTimer_ <= 0.0f) {
            // バースト終了
            burstActive_   = false;
            burstTimer_    = 0.0f;
            burstCooldown_ = 0.0f;
            // バースト直後の硬直を作りたいなら shootCooldown = shootInterval; など
            return;
        }

        if (burstCooldown_ <= 0.0f) {
            const Vector3 myPos     = this->GetWorldPosition();
            const Vector3 targetPos = pPlayer_->GetWorldPosition();
            const Vector3 toTgt     = targetPos - myPos;

            // 正規化（演算子を使わず Normalize を使用）
            Vector3 dir = toTgt.Normalize();
            // もし Normalize がゼロベクトルを返す場合のフォールバック
            if (std::fabs(dir.x) < 1e-6f && std::fabs(dir.y) < 1e-6f && std::fabs(dir.z) < 1e-6f) {
                dir = Vector3{0,0,1};
            }

            fireController_->SpawnStraight(myPos, dir);
            burstCooldown_ = burstInterval_; // 次弾まで 0.1s
        }
        return; // バースト中は通常処理に入らない
    }

    // ===== 通常処理（バースト外）=====
    shootCooldown_ -= dt;

    const Vector3 myPos     = this->GetWorldPosition();
    const Vector3 targetPos = pPlayer_->GetWorldPosition();
    const Vector3 toTgt     = targetPos - myPos;

    // 二乗距離（演算子不要）
    const float dx = toTgt.x, dy = toTgt.y, dz = toTgt.z;
    const float dist2  = dx*dx + dy*dy + dz*dz;
    const float range2 = fireRange_ * fireRange_;
    if (dist2 > range2) return;            // 射程外

    if (shootCooldown_ > 0.0f) return;      // クールダウン中

    // 方向（Normalize 使用）
    Vector3 dir = toTgt.Normalize();
    if (std::fabs(dir.x) < 1e-6f && std::fabs(dir.y) < 1e-6f && std::fabs(dir.z) < 1e-6f) {
        dir = Vector3{0,0,1};
    }

    // 射程の半分以内ならバースト開始、それ以外はホーミング1発
    const float halfRange2 = (fireRange_ * 0.5f) * (fireRange_ * 0.5f);
    if (dist2 <= halfRange2) {
        // バースト開始（すぐ撃ちたいので cooldown 0）
        burstActive_   = true;
        burstTimer_    = 2.0f;     // 2 秒間
        burstInterval_ = 0.1f;     // 0.1 秒間隔
        burstCooldown_ = 0.0f;     // 次フレームですぐ撃つ
        shootCooldown_ = shootInterval_;
    } else {
        // 通常のホーミング射撃（1 発）
        fireController_->SpawnHoming(myPos, dir, static_cast<const BaseObject*>(pPlayer_));
        shootCooldown_ = shootInterval_;
    }
}