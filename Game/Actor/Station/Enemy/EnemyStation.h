#pragma once

// game
#include "../Base/BaseStation.h"
#include "Actor/NPC/Pool/NpcPool.h"
#include "Actor/NPC/EnemyNPC.h"

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

	void SpawnNPC(const Vector3& pos)override;

private:
	/// ===================================================
	///  private variable
	/// ===================================================
	NpcPool<EnemyNPC>pool_;
};
