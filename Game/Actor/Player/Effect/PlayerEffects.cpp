#include "PlayerEffects.h"
#include "audio/Audio.h"

///==========================================================
/// 　初期化
///==========================================================
void PlayerEffects::Init(WorldTransform* transform) {

    starSound_ = Audio::GetInstance()->LoadWave("Resources/starEffect.wav");

    // debri
    debriParticle_[0].reset(ParticleEmitter::CreateParticle("DebriParticle", "debri.obj", 100));

    // star
    starEffect_[0].reset(ParticleEmitter::CreateParticlePrimitive("StarCenterLight", PrimitiveType::Plane, 100));
    starEffect_[1].reset(ParticleEmitter::CreateParticlePrimitive("StarEffect", PrimitiveType::Plane, 100));
    starEffect_[2].reset(ParticleEmitter::CreateParticlePrimitive("StarFrame", PrimitiveType::Plane, 100));

    for (uint32_t i = 0; i < starEffect_.size(); i++) {
        starEffect_[i]->SetFollowingPos(&transform->translation_);
    }

    rushRingEffect_[0].reset(ParticleEmitter::CreateParticlePrimitive("RushEffectRing", PrimitiveType::Ring, 200));
    rushRingEffect_[0]->SetFollowingPos(&transform->translation_);

    // crack
    fallCrack_.reset(ParticleEmitter::CreateParticlePrimitive("Crack", PrimitiveType::Plane, 30));

    // rush
    rushParticle_[0].reset(ParticleEmitter::CreateParticlePrimitive("rushParticle", PrimitiveType::Plane, 800));
    afterGlowEffect_[0].reset(ParticleEmitter::CreateParticle("afterGlowEffect", "Player.obj", 100));
}

///=========================================================
/// 　更新
///==========================================================
void PlayerEffects::Update(const Vector3& position) {
    // ガレキパーティクル
    for (uint32_t i = 0; i < debriParticle_.size(); i++) {
        debriParticle_[i]->SetTargetPosition(position);
        debriParticle_[i]->Update();
        debriParticle_[i]->EditorUpdate();
    }

    // 星パーティクル
    for (uint32_t i = 0; i < starEffect_.size(); i++) {
        starEffect_[i]->Update();
    }

    // fall clack
    fallCrack_->SetTargetPosition(Vector3(position.x, 0.0f, position.z));
    fallCrack_->Update();

    // rush Particle
    rushParticle_[0]->SetTargetPosition(position);
    rushParticle_[0]->Update();
    rushParticle_[0]->EditorUpdate();

    // rush ring
    rushRingEffect_[0]->Update();
    rushRingEffect_[0]->EditorUpdate();

    afterGlowEffect_[0]->SetTargetPosition(position);
    afterGlowEffect_[0]->Update();
    afterGlowEffect_[0]->EditorUpdate();
}

void PlayerEffects::SpecialAttackRenditionInit() {
    // 星パーティクル
    for (uint32_t i = 0; i < starEffect_.size(); i++) {
        starEffect_[i]->Emit();
    }
    Audio::GetInstance()->PlayWave(starSound_, 0.5f);
}

void PlayerEffects::RushAttackEmit() {
    rushParticle_[0]->Emit();
    afterGlowEffect_[0]->Emit();
}

void PlayerEffects::RushAttackRingEffectEmit() {
    rushRingEffect_[0]->Emit();
}

void PlayerEffects::FallEffectRenditionInit() {
    // ガレキパーティクル
    for (uint32_t i = 0; i < debriParticle_.size(); i++) {
        debriParticle_[i]->Emit();
    }
    fallCrack_->Emit();
}