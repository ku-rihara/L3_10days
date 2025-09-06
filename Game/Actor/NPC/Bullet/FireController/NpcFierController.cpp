#include "NpcFierController.h"

void NpcFireController::Init(size_t prewarm){
	straightPool_.Prewarm(prewarm);
	homingPool_.Prewarm(prewarm);
	bullets_.reserve(prewarm * 2);
}

NpcBullet* NpcFireController::Spawn(const Vector3& worldPos, const Vector3& dir, const BaseObject* target){
	switch (currentKind_){
	case BulletKind::Homing:  return SpawnHoming(worldPos, dir, target);
	case BulletKind::Straight:
	default:                   return SpawnStraight(worldPos, dir);
	}
}

NpcBullet* NpcFireController::SpawnStraight(const Vector3& worldPos, const Vector3& dir){
	auto h = straightPool_.Acquire();
	auto* b = h.get();
	b->Init(dir);
	b->SetWorldPosition(worldPos);
	b->Activate();
	bullets_.push_back(std::move(h));
	return b;
}

NpcBullet* NpcFireController::SpawnHoming(const Vector3& worldPos, const Vector3& dir, const BaseObject* target){
	auto h = homingPool_.Acquire();
	auto* hb = static_cast<NpcHomingBullet*>(h.get());
	hb->Init();
	hb->SetDirection(dir);
	hb->SetWorldPosition(worldPos);
	hb->SetTarget(target);
	hb->Activate();
	bullets_.push_back(std::move(h));
	return hb;
}

void NpcFireController::Tick(){
	for (auto& h : bullets_) if (h) h->Update();
	bullets_.erase(
		std::remove_if(bullets_.begin(), bullets_.end(),
					   [](const NpcBulletHandle& h){ return !h || !h->IsAlive(); }),
		bullets_.end()
	);
}