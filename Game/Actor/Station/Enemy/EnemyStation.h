#pragma once

// game
#include "../Base/BaseStation.h"
#include "Actor/NPC/Pool/NpcPool.h"
#include "Actor/NPC/EnemyNPC.h"

class Player;
class NpcFireController;

class EnemyStation final : public BaseStation{
public:
	/// ===================================================
	///  public func
	/// ===================================================
	EnemyStation();
	EnemyStation(const std::string& name = "UnnamedStation");
	~EnemyStation() override;

	void Init() override;
	void Update() override;

	void SpawnNPC(const Vector3& pos) override;

	void SetPlayerPtr(const Player*);
	void CollectTargets(std::vector<const BaseObject*>& out) const override;

private:
	void TryFire();

private:
	/// ===================================================
	///  private variable
	/// ===================================================
	NpcPool<EnemyNPC> pool_;
	std::unique_ptr<NpcFireController> fireController_;
	const Player* pPlayer_ = nullptr;

	float shootCooldown = 0;
	float shootInterval = 3.0f;
	float fireRange_ = 500.0f; //< 射程
};