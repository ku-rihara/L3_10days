#pragma once

/// std
#include <memory>
#include <vector>

/// engine
#include "BaseObject/BaseObject.h"

/// /////////////////////////////////////////////
/// エンジンのエフェクト
/// /////////////////////////////////////////////
class EngineEffect : public BaseObject {
	friend class PlayerEngineEffect;
public:
	EngineEffect(size_t _thisIndex);
	~EngineEffect() override;
	void Init() override;
	void Update() override;
private:
	size_t thisIndex_;
	float uvRotateX_;
	float timer_;
	float randomOffset_;
};


class PlayerEngineEffect {
public:
	PlayerEngineEffect();
	~PlayerEngineEffect();
	void Init();
	void Update();
	void SetPlayer(class Player* _ptr);
private:
	class Player* pPlayer_;

	Vector3 tmpPos_;

	using Element = std::unique_ptr<EngineEffect>;
	std::vector<Element> effects_;
};

