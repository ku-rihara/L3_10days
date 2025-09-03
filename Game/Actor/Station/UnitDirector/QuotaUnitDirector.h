#pragma once
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <cmath>

#include "Actor/Station/UnitDirector/IUnitDirector.h"
#include "Actor/NPC/NPC.h"
#include "Actor/Station/Base/BaseStation.h"

struct UnitDirectorConfig {
	int   squadSize = 4;
	bool  preferSticky = true;
	float defendHoldRadius = 8.0f;
};

class QuotaUnitDirector final : public IUnitDirector {
public:
	explicit QuotaUnitDirector(UnitDirectorConfig cfg = {}) : cfg_(cfg) {}

	// ---- IUnitDirector ----
	int  GetControllableUnitCount(BaseStation* station) const override;
	void AssignDefenseQuota(BaseStation* station, int count, const Vector3& defendAt) override;
	void AssignAttackQuota(BaseStation* station, int count, BaseStation* rivalStation) override;

private:
	enum class Duty { Unknown, Defense, Attack };

	struct PerStation {
		std::unordered_map<NPC*, Duty> duty;
		Vector3 lastDefendPos{};
	};

	std::unordered_map<BaseStation*, PerStation> table_;
	UnitDirectorConfig cfg_{};

private:
	PerStation& Ensure(BaseStation* s);
	static float DistanceSquared(const Vector3& a, const Vector3& b);

	void GcByLive(PerStation& ps, const std::vector<NPC*>& live) const;

	void SplitByDuty(PerStation& ps,
					 const std::vector<NPC*>& live,
					 std::vector<NPC*>& defVec,
					 std::vector<NPC*>& atkVec,
					 std::vector<NPC*>& unkVec) const;

	void ToDefense(PerStation& ps, NPC* npc, BaseStation* owner, const Vector3& defendAt);
	void ToAttack(PerStation& ps, NPC* npc, BaseStation* rival);
};
