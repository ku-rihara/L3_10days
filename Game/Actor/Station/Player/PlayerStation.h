#pragma once

#include "../Base/BaseStation.h"

class PlayerStation final : public BaseStation {
public:
	/// ===================================================
	///  public func
	/// ===================================================
	PlayerStation();
	PlayerStation(const std::string& name = "UnnamedStation");
	virtual ~PlayerStation() override = default;

	void Init() override;
	void Update() override;

	void SpawnNPC()override;
};

