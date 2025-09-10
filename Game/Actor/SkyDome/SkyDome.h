#pragma once

/// engine
#include <memory>

/// game/object
#include "BaseObject/BaseObject.h"
#include "3d/Object3d.h"

class BackgroundObj : public BaseObject {
	friend class SkyDome;
public:
	BackgroundObj();
	~BackgroundObj();
};


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
	void Draw(const ViewProjection& _vp);

private:
	float time_;
	std::unique_ptr<BackgroundObj> bg_ = nullptr;
};




