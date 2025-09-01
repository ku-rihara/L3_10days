#pragma once

#include "NPC.h"

class EnemyNPC final :
	public NPC {
public:
	/// ===================================================
	///  public func
	/// ===================================================
	EnemyNPC();
	~EnemyNPC()override = default;

	void Init()override;
	void Update()override;
};

