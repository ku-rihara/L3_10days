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

	void HormingUpdate();
    void HormingEmitter(const Vector3& pos, const Vector3& rotate);

private:
	std::array<std::unique_ptr<ParticleEmitter>, 5> emitter_;

	//
    std::unique_ptr<ParticleEmitter> npcHomingBulletEmitter_;
	

	class Audio* audio_;
	int soundId_;
};
