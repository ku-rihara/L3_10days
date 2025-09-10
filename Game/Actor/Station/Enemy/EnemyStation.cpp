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

	shootCooldown = shootInterval;
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
	shootCooldown -= dt;

	const Vector3 myPos     = this->GetWorldPosition();
	const Vector3 targetPos = pPlayer_->GetWorldPosition();
	const Vector3 toTgt     = targetPos - myPos;

	// 射程判定（LengthSq が無い場合は直接書く）
	const float dist2  = toTgt.x*toTgt.x + toTgt.y*toTgt.y + toTgt.z*toTgt.z;
	const float range2 = fireRange_ * fireRange_;
	if (dist2 > range2) return;

	if (shootCooldown > 0.0f) return;

	Vector3 dir = toTgt;
	float len = dir.Length();
	if (len > 1e-5f) dir = dir / len;
	else             dir = {0,0,1};

	fireController_->SpawnHoming(myPos, dir, pPlayer_);
	shootCooldown = shootInterval;
}