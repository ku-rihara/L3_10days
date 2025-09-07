#pragma once

#include "3d/ViewProjection.h"

/// utility
#include "utility/ParticleEditor/ParticleEmitter.h"

/// std
#include <array>
#include <cstdint>
#include <memory>
#include <string>

class PlayerEffects {
private:
    /// ===================================================
    /// private various
    /// ===================================================
  
    std::unique_ptr<ParticleEmitter> fallCrack_;
    std::array<std::unique_ptr<ParticleEmitter>, 1> debriParticle_;
    std::array<std::unique_ptr<ParticleEmitter>, 1> rushParticle_;
    std::array<std::unique_ptr<ParticleEmitter>, 1> rushRingEffect_;
    std::array<std::unique_ptr<ParticleEmitter>, 1> afterGlowEffect_;
    std::array<std::unique_ptr<ParticleEmitter>, 3> starEffect_;

    int32_t starSound_;

public:
    PlayerEffects()=default;
    ~PlayerEffects() = default;
    /// ===================================================
    ///  public method
    /// ===================================================

    ///* 初期化、更新、描画
    void Init(WorldTransform* transform);
    void Update(const Vector3& position);
   
    void SpecialAttackRenditionInit();
    void FallEffectRenditionInit();
    void RushAttackEmit();
    void RushAttackRingEffectEmit();

    /// ====================================================================
    /// Editor
    /// ====================================================================

    /// =========================================================================================
    /// getter
    /// =========================================================================================

    /// =========================================================================================
    /// setter
    /// =========================================================================================
};