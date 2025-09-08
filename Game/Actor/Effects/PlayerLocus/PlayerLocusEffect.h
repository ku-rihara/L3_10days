#pragma once

/// std
#include <memory>
#include <vector>

/// engine
#include"utility/ParticleEditor/ParticleEmitter.h"

class PlayerLocusEffect {
public:
	PlayerLocusEffect();
	~PlayerLocusEffect();
	void Init(class Player* _ptr);
	void Update();
private:
	class Player* pPlayer_;
	Vector3 playerPos_;
	using UEmitter = std::unique_ptr<ParticleEmitter>;
	std::vector<UEmitter> emitters_;
};

