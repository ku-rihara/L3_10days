#pragma once
#include "BaseObject/BaseObject.h"

class NPC : public BaseObject{
public:
	
	NPC() = default;
	~NPC() = default;
	void Init() override;
	void Update() override;

private:
};
