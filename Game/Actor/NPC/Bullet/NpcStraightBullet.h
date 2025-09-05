#pragma once
#include "NpcBullet.h"

class NpcStraightBullet final:
		public NpcBullet{
public:
	/// ===================================================
	/// public func
	/// ===================================================
	NpcStraightBullet() = default;
	~NpcStraightBullet() override = default;

	void Init() override;
	void Update() override;
	void ShootInit();

	//=== accessor ========================================
	void SetTarget(const BaseObject* target);

private:
	/// ===================================================
	/// private func
	/// ===================================================
	void BindParms()override;

private:
	/// ===================================================
	/// private variable
	/// ===================================================
	const BaseObject* target_ = nullptr;
};