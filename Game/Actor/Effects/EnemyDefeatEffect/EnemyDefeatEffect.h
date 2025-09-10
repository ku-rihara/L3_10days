#pragma once

/// std
#include <memory>
#include <vector>

/// engine
#include "utility/ParticleEditor/ParticleEmitter.h"

class EnemyDefeatEffect {
public:
	EnemyDefeatEffect(float _lifeTime);
	EnemyDefeatEffect(const EnemyDefeatEffect&) = delete;
	EnemyDefeatEffect& operator=(const EnemyDefeatEffect&) = delete;
	EnemyDefeatEffect(EnemyDefeatEffect&&) = default;
	EnemyDefeatEffect& operator=(EnemyDefeatEffect&&) = default;
	~EnemyDefeatEffect();

	void Update();
	float GetLifeTime() const;
	void SetEmitPosition(const Vector3& _position);
private:
	using UEmitter = std::unique_ptr<ParticleEmitter>;
	std::vector<UEmitter> emitters_;
	float thisLifeTime_;
	Vector3 emitPosition_;
};

class ExplosionEffectPool {
	ExplosionEffectPool();
	~ExplosionEffectPool();
public:
	static ExplosionEffectPool& GetInstance();

	void Update();

	void EmitExplosion(const Vector3& position, float lifeTime = 1.0f);

private:
	std::vector<EnemyDefeatEffect> effects_;
};
