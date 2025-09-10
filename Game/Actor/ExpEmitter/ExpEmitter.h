#pragma once

/// engine
#include "utility/ParticleEditor/ParticleEmitter.h"
#include "Vector3.h"

/// std
#include <memory>
#include <array>

class ExpEmitter final {
	ExpEmitter();
	~ExpEmitter();
public:

	static ExpEmitter* GetInstance();

	void Init();
	void Update();
	void Emit(const Vector3& pos);

private:
	std::array<std::unique_ptr<ParticleEmitter>, 5> emitter_;

	class Audio* audio_;
	int soundId_;
};
