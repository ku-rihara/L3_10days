#pragma once
#include "NpcBullet.h"

class NpcHomingBullet:public NpcBullet{
public:
	/// ===================================================
	/// public func
	/// ===================================================
	NpcHomingBullet() = default;
	virtual ~NpcHomingBullet()override = default;

	void Init()override;
	void Update() override;

private:
	/// ===================================================
	/// private func
	/// ===================================================

};