#include "PlayerStation.h"

#include "Actor/NPC/PlayerNPC.h"
#include "Actor/NPC/Pool/NpcPool.h"
#include "Frame/Frame.h"

#include "imgui.h"

PlayerStation::PlayerStation() { BaseStation::SetFaction(FactionType::Ally); }

PlayerStation::PlayerStation(const std::string& name) :
	BaseStation(name) {
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

	BaseStation::Init();
}

/// ===================================================
/// 更新
/// ===================================================
void PlayerStation::Update() {
	float dt = Frame::DeltaTime();
	//スポーン
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

	if (static_cast<int>(spawned_.size()) >= maxConcurrentUnits_) return;

	auto npc = pool_.Acquire();
	npc->Init();
	npc->SetFaction(FactionType::Ally);

	if (auto* rival = this->GetRivalStation()){
		npc->SetTargetProvider(this);
	}

	npc->SetWorldPosition(spawnPos);
	npc->SetTarget(GetRivalStation());

	spawned_.push_back(std::move(npc));
	currentTime_ = 0.0f;
}