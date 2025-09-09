#pragma once

/// engine
#include "utility/ParticleEditor/ParticleEmitter.h"
#include "Vector3.h"

/// std
#include <memory>
#include <array>

class ExpEmitter {
public:
    ExpEmitter()  = default;
    ~ExpEmitter() = default;

    void Init();
    void Update();
    void Emit(const Vector3& pos);

    static ExpEmitter* GetInstance();

private:
    std::array<std::unique_ptr<ParticleEmitter>, 5> emitter_;
};
