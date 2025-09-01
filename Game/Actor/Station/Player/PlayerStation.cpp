#include "PlayerStation.h"

#include "Actor/NPC/PlayerNPC.h"
#include "Frame/Frame.h"

#include "imgui.h"

PlayerStation::PlayerStation() { BaseStation::SetFaction(FactionType::Enemy); }

PlayerStation::PlayerStation(const std::string& name) :
	BaseStation(name) {
	BaseStation::SetFaction(FactionType::Enemy);
}

/// ===================================================
/// 初期化
/// ===================================================
void PlayerStation::Init() {
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
		SpawnNPC();
	}

	for (auto& enemy : spawned_) {
		enemy->Update();
	}

	BaseStation::Update();
}

/// ===================================================
/// npcのスポーン
/// ===================================================
void PlayerStation::SpawnNPC() {
	//auto rival = GetRivalStation();
	//if (!rival)return;

	//上限チェック
	if (static_cast<int>(spawned_.size()) >= maxConcurrentUnits_)return;

	//1体スポーン
	auto npc = std::make_unique<PlayerNPC>();
	npc->Init();
	Vector3 spawnOffset = { 1.0f,1.0f,1.0f };
	const Vector3 worldSpawn = baseTransform_.translation_ + spawnOffset;
	npc->SetWorldPosition(worldSpawn);

	spawned_.push_back(std::move(npc));

	currentTime_ = 0.0f;//リセット
}
