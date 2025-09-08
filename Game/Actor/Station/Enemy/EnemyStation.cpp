#include "EnemyStation.h"

#include "Actor/NPC/EnemyNPC.h"
#include "Actor/NPC/Pool/NpcPool.h"
#include "Frame/Frame.h"
#include "Actor/Boundary/Boundary.h"
#include "Actor/Player/Player.h"

#include "imgui.h"

EnemyStation::EnemyStation() { BaseStation::SetFaction(FactionType::Enemy); }

EnemyStation::EnemyStation(const std::string& name) :
	BaseStation(name) {
	BaseStation::SetFaction(FactionType::Enemy);
}

EnemyStation::~EnemyStation() { spawned_.clear(); }

/// ===================================================
/// 初期化
/// ===================================================
void EnemyStation::Init() {
	obj3d_.reset(Object3d::CreateModel("EnemyBase.obj"));
	BaseObject::Init();
	obj3d_->transform_.parent_ = &baseTransform_;

	globalParam_ = GlobalParameter::GetInstance();

	BaseStation::Init();
}

/// ===================================================
/// 更新
/// ===================================================
void EnemyStation::Update() {
	float dt = Frame::DeltaTime();
	//スポーン
	currentTime_ += dt;
	if (currentTime_ >= spawnInterbal_) {
		SpawnNPC(GetWorldPosition());
	}

	for (auto& enemy:spawned_) {
		enemy->Update();
	}

	BaseStation::Update();
}

/// ===================================================
/// npcのスポーン

void EnemyStation::SpawnNPC(const Vector3& pos) {
	if (static_cast<int>(spawned_.size()) >= maxConcurrentUnits_) return;

	auto npc = pool_.Acquire();
	npc->Init();
	npc->SetFaction(FactionType::Enemy);

	if (auto* rival = this->GetRivalStation()) {
		npc->SetTargetProvider(this);
	}

	npc->SetWorldPosition(pos);
	npc->SetTarget(GetRivalStation());

	spawned_.push_back(std::move(npc));
	currentTime_ = 0.0f;
}

void EnemyStation::SetPlayerPtr(const Player* player) { pPlayer_ = player; }

void EnemyStation::CollectTargets(std::vector<const BaseObject*>& out) const {
	BaseStation::CollectTargets(out);

	// 自分が Enemy で、プレイヤーが指定されていれば攻撃候補に追加
	if (GetFactionType() == FactionType::Enemy && pPlayer_) {
		out.push_back(static_cast<const BaseObject*>(pPlayer_));
	}
}