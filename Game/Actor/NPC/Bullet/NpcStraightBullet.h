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

	void Init() override;		//< モデルを変えるため初期化をoverride
};