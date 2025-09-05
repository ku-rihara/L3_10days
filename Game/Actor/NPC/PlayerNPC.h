#pragma once

#include "NPC.h"

class PlayerNPC final :
	public NPC {
public:
	/// ===================================================
	///  public func
	/// ===================================================
	PlayerNPC();
	~PlayerNPC()override;

	void Init()override;
	void Update()override;
};