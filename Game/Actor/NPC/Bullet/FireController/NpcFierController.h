#pragma once
#include <vector>
#include <algorithm>
#include "../Pool/NpcBulletPool.h"
#include "../NpcBullet.h"

class NpcFireController {
public:
	void Init(size_t prewarm = 128);

	// dir は未正規化でもOK（NpcBullet側で正規化）
	NpcBullet* Spawn(const Vector3& worldPos, const Vector3& dir);

	void Tick();

private:
	NpcBulletPool<NpcBullet> pool_;
	std::vector<NpcBulletHandle> bullets_;
};
