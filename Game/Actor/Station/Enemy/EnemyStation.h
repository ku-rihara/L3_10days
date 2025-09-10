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

	float fireRange_ = 700.0f; //< 射程

	// 通常射撃用
	float shootCooldown_  = 0.0f;
	float shootInterval_  = 1.0f;

	// バースト射撃用
	bool  burstActive_    = false;
	float burstTimer_     = 0.0f;   // 残り時間
	float burstInterval_  = 0.1f;   // 弾間隔
	float burstCooldown_  = 0.0f;   // 次弾までの残り
};