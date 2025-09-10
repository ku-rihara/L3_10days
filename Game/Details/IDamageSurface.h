#pragma once
#include "Physics/Contact.h"

class IDamageSurface {
public:
	virtual ~IDamageSurface() = default;
	virtual bool OnBulletImpact(const Contact& c, float damage) = 0;
};