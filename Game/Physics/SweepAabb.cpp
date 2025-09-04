#include "SweepAabb.h"

void Sweep::Slab1D(float p0, float p1, float bmin, float bmax,
				   float& tEnter, float& tExit, int& axisEnter, int axisId) {
	float v = p1 - p0;
	if (std::fabs(v) < 1e-8f) {
		if (p0 < bmin || p0 > bmax) { tEnter = 1.0f; tExit = 0.0f; }
		return;
	}
	float inv = 1.0f / v;
	float t0 = (bmin - p0) * inv;
	float t1 = (bmax - p0) * inv;
	if (t0 > t1) std::swap(t0, t1);

	if (t0 > tEnter) { tEnter = t0; axisEnter = axisId; }
	if (t1 < tExit) { tExit = t1; }
}

std::optional<Contact> Sweep::SegmentSphereVsAabb(const Vector3& p0, const Vector3& p1,
												  float radius, const AABB& box) {
	AABB expanded{
		{ box.min.x - radius, box.min.y - radius, box.min.z - radius },
		{ box.max.x + radius, box.max.y + radius, box.max.z + radius }
	};

	float tEnter = 0.0f, tExit = 1.0f;
	int axisEnter = -1;

	Slab1D(p0.x, p1.x, expanded.min.x, expanded.max.x, tEnter, tExit, axisEnter, 0);
	Slab1D(p0.y, p1.y, expanded.min.y, expanded.max.y, tEnter, tExit, axisEnter, 1);
	Slab1D(p0.z, p1.z, expanded.min.z, expanded.max.z, tEnter, tExit, axisEnter, 2);

	if (tEnter > tExit || tEnter < 0.0f || tEnter > 1.0f) return std::nullopt;

	Vector3 v = p1 - p0;
	Vector3 pHitExp = p0 + v * tEnter;

	Contact c{};
	c.t = tEnter;
	c.point = pHitExp;

	switch (axisEnter) {
		case 0: // x面
			if (std::fabs(pHitExp.x - expanded.min.x) < std::fabs(pHitExp.x - expanded.max.x)) {
				c.normal = { -1, 0, 0 };
				c.point.x = box.min.x;
			} else {
				c.normal = { +1, 0, 0 };
				c.point.x = box.max.x;
			}
			c.point.y = std::clamp(pHitExp.y, box.min.y, box.max.y);
			c.point.z = std::clamp(pHitExp.z, box.min.z, box.max.z);
			break;
		case 1: // y面
			if (std::fabs(pHitExp.y - expanded.min.y) < std::fabs(pHitExp.y - expanded.max.y)) {
				c.normal = { 0, -1, 0 };
				c.point.y = box.min.y;
			} else {
				c.normal = { 0, +1, 0 };
				c.point.y = box.max.y;
			}
			c.point.x = std::clamp(pHitExp.x, box.min.x, box.max.x);
			c.point.z = std::clamp(pHitExp.z, box.min.z, box.max.z);
			break;
		case 2: // z面
			if (std::fabs(pHitExp.z - expanded.min.z) < std::fabs(pHitExp.z - expanded.max.z)) {
				c.normal = { 0, 0, -1 };
				c.point.z = box.min.z;
			} else {
				c.normal = { 0, 0, +1 };
				c.point.z = box.max.z;
			}
			c.point.x = std::clamp(pHitExp.x, box.min.x, box.max.x);
			c.point.y = std::clamp(pHitExp.y, box.min.y, box.max.y);
			break;
		default:
			return std::nullopt;
	}
	return c;
}