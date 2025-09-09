#include"ExpEmitter.h"

ExpEmitter* ExpEmitter::GetInstance() {
    static ExpEmitter instance;
    return &instance;
}
void ExpEmitter::Init() {
    emitter_[2].reset(ParticleEmitter::CreateParticlePrimitive("Explosion1", PrimitiveType::Box, 500));
    emitter_[0].reset(ParticleEmitter::CreateParticlePrimitive("Explosion2", PrimitiveType::Box, 500));
    emitter_[3].reset(ParticleEmitter::CreateParticlePrimitive("Explosion3", PrimitiveType::Box, 500));
    emitter_[4].reset(ParticleEmitter::CreateParticlePrimitive("Explosion5", PrimitiveType::Box, 500));
    emitter_[1].reset(ParticleEmitter::CreateParticlePrimitive("Explosion4", PrimitiveType::Box, 500));
}

void ExpEmitter::Update() {
    for (auto& emit : emitter_) {
        emit->Update();
    }
}

void ExpEmitter::Emit(const Vector3& pos) {
    for (auto& emit: emitter_) {
        emit->SetTargetPosition(pos);
        emit->Emit();
    }
}