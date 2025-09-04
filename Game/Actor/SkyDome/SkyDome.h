#pragma once

/// game/object
#include "BaseObject/BaseObject.h"
#include "3d/Object3d.h"


/// ////////////////////////////////////////////////////////////
/// 天球
/// ////////////////////////////////////////////////////////////
class SkyDome : public BaseObject {
public:
	/// ==========================================================
	/// public : methods
	/// ==========================================================

	SkyDome() = default;
	~SkyDome() override = default;

	void Init() override;
	void Update() override;

private:
};

