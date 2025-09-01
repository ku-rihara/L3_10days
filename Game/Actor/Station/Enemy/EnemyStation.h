#pragma once
#include "../Base/BaseStation.h"

class EnemyStation : public BaseStation {
public:
	/// ===================================================
	///  public func
	/// ===================================================
	virtual ~EnemyStation()override = default;

	virtual void Init()override;
	virtual void Update()override;
	
private:
	/// ===================================================
	///  public variaus
	/// ===================================================

};