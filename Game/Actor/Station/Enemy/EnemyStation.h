#pragma once

// game
#include "../Base/BaseStation.h"

#include <string>

class EnemyStation final : public BaseStation{
public:
	/// ===================================================
	///  public func
	/// ===================================================
	EnemyStation();
	EnemyStation(const std::string& name = "UnnamedStation");
	virtual ~EnemyStation() override = default;

	void Init() override;
	void Update() override;

public:
	/// ===================================================
	///  private variable
	/// ===================================================
};
