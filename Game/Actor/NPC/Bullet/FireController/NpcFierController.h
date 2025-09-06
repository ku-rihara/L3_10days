#pragma once
#include <vector>
#include "../Pool/NpcBulletPool.h"
#include "../NpcBullet.h"
#include "../NpcStraightBullet.h"
#include "../NpcHomingBullet.h"

class NpcFireController {
public:
	enum class BulletKind { Straight, Homing };

	void Init(size_t prewarm = 128);

	// 現在の弾種でスポーン（Homing の場合は target を渡すと追尾開始）
	NpcBullet* Spawn(const Vector3& worldPos, const Vector3& dir, const BaseObject* target = nullptr);

	// 明示的に撃ち分け（ワンショット指定）
	NpcBullet* SpawnStraight(const Vector3& worldPos, const Vector3& dir);
	NpcBullet* SpawnHoming (const Vector3& worldPos, const Vector3& dir, const BaseObject* target);

	// 現在のデフォルト弾種を切替
	void SetKind(BulletKind k) noexcept { currentKind_ = k; }
	BulletKind GetKind() const noexcept { return currentKind_; }

	void Tick();

private:
	// プール（弾種ごと）
	NpcBulletPool<NpcStraightBullet> straightPool_;
	NpcBulletPool<NpcHomingBullet>   homingPool_;

	std::vector<NpcBulletHandle> bullets_;
	BulletKind currentKind_ = BulletKind::Straight;
};