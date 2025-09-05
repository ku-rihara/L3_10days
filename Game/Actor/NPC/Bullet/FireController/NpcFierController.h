#pragma once
#include <vector>
#include "../Pool/NpcBulletPool.h"
#include "../NpcBullet.h"

class NpcFireController {
public:
	void Init(size_t prewarm = 128);
	NpcBullet* Spawn(const Vector3& worldPos, const Vector3& dir);
	void Tick();
private:
	NpcBulletPool<NpcBullet> pool_;
	std::vector<NpcBulletHandle> bullets_;
};