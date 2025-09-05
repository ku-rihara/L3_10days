#include "NpcFierController.h"

void NpcFireController::Init(size_t prewarm){
	pool_.Prewarm(prewarm);
	bullets_.reserve(prewarm);
}

NpcBullet* NpcFireController::Spawn(const Vector3& worldPos, const Vector3& dir){
	auto h = pool_.Acquire();
	NpcBullet* b = h.get();
	b->Init(dir);
	b->SetWorldPosition(worldPos);
	bullets_.push_back(std::move(h));
	return b;
}

void NpcFireController::Tick(){
	for (auto& h : bullets_) if (h) h->Update();
	bullets_.erase(
				   std::remove_if(bullets_.begin(),bullets_.end(),
								  [](const NpcBulletHandle& h){ return !h || !h->IsAlive(); }),
				   bullets_.end()
				  );
}