#pragma once

/// engine
#include "BaseObject/BaseObject.h"

class Hunger : public BaseObject {
public:
	Hunger();
	~Hunger();
	void Init() override;
	void Update() override;
private:
};

