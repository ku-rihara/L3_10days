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
	void DrawBG(const ViewProjection& _vp);
	void DrawSkyDome(const ViewProjection& _vp);

	Object3d* GetObj3d() { return obj3d_.get(); }

private:
	float time_;
	std::unique_ptr<BackgroundObj> bg_ = nullptr;
};





