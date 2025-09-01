#include "EnemyStation.h"

#include "Actor/NPC/EnemyNPC.h"
#include "Frame/Frame.h"

#include "imgui.h"

EnemyStation::EnemyStation() { BaseStation::SetFaction(FactionType::Enemy); }

EnemyStation::EnemyStation(const std::string& name) :
	BaseStation(name) {
	BaseStation::SetFaction(FactionType::Enemy);
}

/// ===================================================
/// 初期化
/// ===================================================
void EnemyStation::Init() {
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
		SpawnNPC();
	}

	for (auto& enemy:spawned_) {
		enemy->Update();
	}

	BaseStation::Update();
}

/// ===================================================
/// npcのスポーン
/// ===================================================
void EnemyStation::SpawnNPC() {
	auto rival = wRivalStation_.lock();
	//if (!rival)return;

	//上限チェック
	if (static_cast<int>(spawned_.size()) >= maxConcurrentUnits_)return;

	//1体スポーン
	auto npc = std::make_unique<EnemyNPC>();
	npc->Init();
	Vector3 spawnOffset = { 1.0f,1.0f,1.0f };
	const Vector3 worldSpawn = baseTransform_.translation_ + spawnOffset;
	npc->SetWorldPosition(worldSpawn);

	spawned_.push_back(std::move(npc));

	currentTime_ = 0.0f;//リセット
}
