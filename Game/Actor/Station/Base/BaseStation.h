#pragma once

#include "BaseObject/BaseObject.h"

class BaseStation:public BaseObject{
public:
	/// ===================================================
	///  public func
	/// ===================================================

	virtual ~BaseStation()override = default;

	virtual void Init()override;
	virtual void Update()override;
	
private:
	/// ===================================================
	///  public variaus
	/// ===================================================

};
