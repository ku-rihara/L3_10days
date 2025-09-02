#include "NpcNavigator.h"
#include "Actor/Boundary/Boundary.h" // Hole
#include <algorithm>
#include <limits>
#include <cmath>

static inline Vector3 NormalizeSafe3(const Vector3& v, const Vector3& fb = { 0,0,1 }) {
	float l = v.Length();
	return (l > 1e-4f) ? (v * (1.0f / l)) : fb;
}

void NpcNavigator::Reset(const Vector3& orbitCenter) noexcept {
	state_ = State::Orbit;
	orbitCenter_ = orbitCenter;
	orbitAngle_ = 0.0f;
	retargetCooldown_ = 0.0f;
	holeIndex_ = -1; holePos_ = {}; holeRadius_ = 0.0f;
}

Vector3 NpcNavigator::Tick(float dt,
						   const Vector3& npcPos,
						   const Vector3& tgtPos,
						   const std::vector<Hole>& holes) {
	if (retargetCooldown_ > 0.0f) retargetCooldown_ -= dt;

	auto hasValidHole = [&]() {
		for (const auto& h : holes) if (h.radius >= cfg_.minHoleRadius) return true;
		return false;
	};
	auto invalidateHole = [&]() { holeIndex_ = -1; holePos_ = {}; holeRadius_ = 0.0f; };
	auto tryPickHole = [&]() {
		if (retargetCooldown_ > 0.0f) return;
		int best = SelectBestHole(holes, npcPos, tgtPos);
		if (best >= 0) {
			holeIndex_ = best;
			holePos_ = holes[best].position;
			holeRadius_ = holes[best].radius;
			state_ = State::ToHole;
			retargetCooldown_ = cfg_.retargetInterval;
		} else {
			StartOrbit(npcPos);
		}
	};

	switch (state_) {
		case State::Orbit:
			{
				if (hasValidHole()) tryPickHole();
				orbitAngle_ += cfg_.orbitAngularSpd * dt;
				Vector3 desired = orbitCenter_ + Vector3(std::cos(orbitAngle_), 0.0f, std::sin(orbitAngle_)) * cfg_.orbitRadius;
				Vector3 dir = NormalizeSafe3(desired - npcPos);
				return dir * cfg_.speed * dt;
			}
		case State::ToHole:
			{
				bool ok = false;
				if (holeIndex_ >= 0 && holeIndex_ < (int)holes.size()) {
					const auto& h = holes[holeIndex_];
					ok = (h.radius >= cfg_.minHoleRadius);
					holePos_ = h.position;
					holeRadius_ = h.radius;
				}
				if (!ok) { invalidateHole(); tryPickHole(); if (state_ == State::Orbit) return { 0,0,0 }; }

				Vector3 toHole = holePos_ - npcPos;
				float   distH = toHole.Length();
				if (distH <= (std::max)(1.0f, holeRadius_ * cfg_.passFrac)) { state_ = State::ToTarget; invalidateHole(); return { 0,0,0 }; }
				toHole = (distH > 1e-4f) ? (toHole * (1.0f / distH)) : Vector3{ 0,0,1 };
				return toHole * cfg_.speed * dt;
			}
		case State::ToTarget:
			{
				if (hasValidHole()) { tryPickHole(); if (state_ == State::ToHole) return { 0,0,0 }; }
				Vector3 toTgt = tgtPos - npcPos;
				float   dist = toTgt.Length();
				toTgt = (dist > 1e-4f) ? (toTgt * (1.0f / dist)) : Vector3{ 0,0,1 };
				float speedScale = 1.0f;
				if (dist < cfg_.arriveSlowRadius) speedScale = dist / (std::max)(1e-3f, cfg_.arriveSlowRadius);
				return toTgt * cfg_.speed * speedScale * dt;
			}
	}
	return { 0,0,0 };
}

int NpcNavigator::SelectBestHole(const std::vector<Hole>& holes,
								 const Vector3& npcPos,
								 const Vector3& tgtPos) const {
	if (holes.empty()) return -1;

	const Vector3 toTgt = tgtPos - npcPos;
	float toTgtLen = toTgt.Length();
	Vector3 toTgtDir = (toTgtLen > 1e-4f) ? (toTgt * (1.0f / toTgtLen)) : Vector3{ 0,0,1 };

	int   bestIdx = -1;
	float bestCost = (std::numeric_limits<float>::max)();

	for (int i = 0; i < (int)holes.size(); ++i) {
		const Hole& h = holes[i];
		if (h.radius < cfg_.minHoleRadius) continue;

		Vector3 toHole = h.position - npcPos;
		float   thLen = toHole.Length();
		if (thLen < 1e-4f) continue;
		Vector3 toHoleDir = toHole * (1.0f / thLen);

		float cosA = Vector3::Dot(toHoleDir, toTgtDir);
		if (cosA < 0.0f) continue;

		float cost = thLen + (tgtPos - h.position).Length() * 1.05f;
		if (cost < bestCost) { bestCost = cost; bestIdx = i; }
	}
	return bestIdx;
}

void NpcNavigator::StartOrbit(const Vector3& center) noexcept {
	state_ = State::Orbit;
	orbitCenter_ = center;
	orbitAngle_ = 0.0f;
	retargetCooldown_ = cfg_.retargetInterval;
}
