#pragma once
#include <optional>
#include <algorithm>
#include <cmath>

#include "Vector3.h"
#include "Box.h"
#include "Physics/Contact.h"

namespace Sweep {

	void Slab1D(float p0, float p1, float bmin, float bmax,
					   float& tEnter, float& tExit, int& axisEnter, int axisId);

	std::optional<Contact>
		SegmentSphereVsAabb(const Vector3& p0, const Vector3& p1,
							float radius, const AABB& box);

}