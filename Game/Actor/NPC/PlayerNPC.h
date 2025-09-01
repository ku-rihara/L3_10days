#pragma once

#include "NPC.h"

class PlayerNPC final :
	public NPC {
public:
	/// ===================================================
	///  public func
	/// ===================================================
	PlayerNPC();
	~PlayerNPC()override = default;

	void Init()override;
	void Update()override;
};

