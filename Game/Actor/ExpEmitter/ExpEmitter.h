#pragma once

/// std
#include <memory>
#include <vector>

/// engine
#include "utility/ParticleEditor/ParticleEmitter.h"
#include <array>

class ExpEmitter {
public:
    ExpEmitter()  = default;
    ~ExpEmitter() = default;

    static ExpEmitter* GetInstance();

private:
    std::array<std::unique_ptr<ParticleEmitter>, 5> emitter_;
};
