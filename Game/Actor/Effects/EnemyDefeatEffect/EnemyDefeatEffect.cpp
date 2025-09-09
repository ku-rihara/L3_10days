#include "EnemyDefeatEffect.h"

EnemyDefeatEffect::EnemyDefeatEffect(float _lifeTime)
	: thisLifeTime_(_lifeTime) {

	// Particle初期化(ファイル名,形状,Particle数上限)
	emitters_.emplace_back(ParticleEmitter::CreateParticlePrimitive("Explosion1", PrimitiveType::Box, 500));
	emitters_.emplace_back(ParticleEmitter::CreateParticlePrimitive("Explosion2", PrimitiveType::Box, 500));
	emitters_.emplace_back(ParticleEmitter::CreateParticlePrimitive("Explosion3", PrimitiveType::Box, 500));
	emitters_.emplace_back(ParticleEmitter::CreateParticlePrimitive("Explosion4", PrimitiveType::Box, 500));
	emitters_.emplace_back(ParticleEmitter::CreateParticlePrimitive("Explosion5", PrimitiveType::Box, 500));
}

EnemyDefeatEffect::~EnemyDefeatEffect() = default;


void EnemyDefeatEffect::Update() {
	//// Particle更新
	for (int i = 0; i < emitters_.size(); i++) {
		emitters_[i]->Update(); // 更新
		emitters_[i]->Emit(); // 発射
		emitters_[i]->SetTargetPosition(emitPosition_);
	}
}

float EnemyDefeatEffect::GetLifeTime() const {
	return thisLifeTime_;
}

void EnemyDefeatEffect::SetEmitPosition(const Vector3& _position) {
	emitPosition_ = _position;
}



ExplosionEffectPool::ExplosionEffectPool() {}
ExplosionEffectPool::~ExplosionEffectPool() = default;

ExplosionEffectPool& ExplosionEffectPool::GetInstance() {
	static ExplosionEffectPool instance;
	return instance;
}

void ExplosionEffectPool::Update() {
	for (auto& ef : effects_) {
		ef.Update();
	}

	/// lifeTimeが過ぎたものを消す
	auto it = std::remove_if(effects_.begin(), effects_.end(), [](const EnemyDefeatEffect& ef) {
		return ef.GetLifeTime() <= 0.0f;
		});
	effects_.erase(it, effects_.end());
}

void ExplosionEffectPool::EmitExplosion(const Vector3& position, float lifeTime) {
	EnemyDefeatEffect effect(lifeTime);
	effect.SetEmitPosition(position);
	effects_.emplace_back(std::move(effect));
}

