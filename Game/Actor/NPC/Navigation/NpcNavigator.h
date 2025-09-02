#pragma once
#include <vector>
#include "Vector3.h"
struct Hole;

struct NpcNavConfig {
	float speed = 18.0f;
	float arriveSlowRadius = 3.0f;
	float minHoleRadius = 8.0f;
	float passFrac = 0.25f;
	float retargetInterval = 0.2f;
	float orbitRadius = 20.0f;
	float orbitAngularSpd = 1.2f; // rad/sec
};

class NpcNavigator {
public:
	enum class State { Orbit, ToHole, ToTarget };

	explicit NpcNavigator(const NpcNavConfig& cfg = {}) : cfg_(cfg) {}
	void   Reset(const Vector3& orbitCenter) noexcept;

	// 望ましい移動ベクトル（速度×dt）を返す
	Vector3 Tick(float dt,
				 const Vector3& npcPos,
				 const Vector3& tgtPos,
				 const std::vector<Hole>& holes);

	void   StartOrbit(const Vector3& center) noexcept;
	State  GetState() const noexcept { return state_; }

private:
	int  SelectBestHole(const std::vector<Hole>& holes,
						const Vector3& npcPos,
						const Vector3& tgtPos) const;

private:
	NpcNavConfig cfg_;
	State  state_ = State::Orbit;

	// 穴追跡
	int     holeIndex_ = -1;
	Vector3 holePos_{};
	float   holeRadius_ = 0.0f;
	float   retargetCooldown_ = 0.0f;

	// 旋回
	Vector3 orbitCenter_{};
	float   orbitAngle_ = 0.0f;
};
