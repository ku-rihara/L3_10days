#pragma once
#include "NpcBullet.h"

/// <summary>
/// 境界を壊す専用の弾
/// </summary>
class BoundaryBreakerBullet :
	public NpcBullet {
public:
	//===================================================================*/
	//					public methods
	//===================================================================*/
	BoundaryBreakerBullet() = default;
	~BoundaryBreakerBullet() override = default;

	void Init()override;
	void Hit()override;
};

