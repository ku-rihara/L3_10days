#include "QuotaUnitDirector.h"

// ===== util =====
float QuotaUnitDirector::DistanceSquared(const Vector3& a, const Vector3& b) {
	const float dx = a.x - b.x, dy = a.y - b.y, dz = a.z - b.z;
	return dx * dx + dy * dy + dz * dz;
}

QuotaUnitDirector::PerStation& QuotaUnitDirector::Ensure(BaseStation* s) {
	return table_[s];
}

void QuotaUnitDirector::GcByLive(PerStation& ps, const std::vector<NPC*>& live) const {
	std::unordered_map<NPC*, char> liveSet;
	liveSet.reserve(live.size());
	for (auto* n : live) liveSet[n] = 1;

	for (auto it = ps.duty.begin(); it != ps.duty.end();) {
		if (!liveSet.count(it->first)) it = ps.duty.erase(it);
		else ++it;
	}
}

void QuotaUnitDirector::SplitByDuty(PerStation& ps,
									const std::vector<NPC*>& live,
									std::vector<NPC*>& defVec,
									std::vector<NPC*>& atkVec,
									std::vector<NPC*>& unkVec) const {
	defVec.clear(); atkVec.clear(); unkVec.clear();
	defVec.reserve(live.size());
	atkVec.reserve(live.size());
	unkVec.reserve(live.size());

	for (auto* n : live) {
		auto it = ps.duty.find(n);
		Duty d = (it != ps.duty.end()) ? it->second : Duty::Unknown;
		if (d == Duty::Defense) defVec.push_back(n);
		else if (d == Duty::Attack) atkVec.push_back(n);
		else unkVec.push_back(n);
	}
}

int QuotaUnitDirector::GetControllableUnitCount(BaseStation* station) const {
	if (!station) return 0;
	return static_cast<int>(station->GetLiveNpcs().size());
}

void QuotaUnitDirector::ToDefense(PerStation& ps, NPC* npc, BaseStation* owner, const Vector3& defendAt) {
	if (!npc || !owner) return;
	const Vector3 pos = npc->GetWorldPosition();
	if (std::sqrt(DistanceSquared(pos, defendAt)) <= cfg_.defendHoldRadius) {
		npc->SetTarget(nullptr);  // その場オービット（ターゲット無し）:contentReference[oaicite:2]{index=2}
	} else {
		npc->SetTarget(owner);    // 拠点へ集合（防衛）:contentReference[oaicite:3]{index=3}
	}
	ps.duty[npc] = Duty::Defense;
}

void QuotaUnitDirector::ToAttack(PerStation& ps, NPC* npc, BaseStation* rival) {
	if (!npc) return;
	npc->SetTarget(rival);        // Rival 拠点へ突撃（既存運用と同じ）:contentReference[oaicite:4]{index=4}
	ps.duty[npc] = Duty::Attack;
}

void QuotaUnitDirector::AssignDefenseQuota(BaseStation* station, int count, const Vector3& defendAt) {
	if (!station) return;
	auto& ps = Ensure(station);
	ps.lastDefendPos = defendAt;

	std::vector<NPC*> live = station->GetLiveNpcs();
	GcByLive(ps, live);

	std::vector<NPC*> defC, atkC, unkC;
	SplitByDuty(ps, live, defC, atkC, unkC);

	std::vector<NPC*> chosen; chosen.reserve(count);
	for (auto* n : defC) {
		if ((int)chosen.size() >= count) break;
		chosen.push_back(n);
	}

	auto fillNear = [&](std::vector<NPC*>& pool) {
		std::sort(pool.begin(), pool.end(), [&](NPC* a, NPC* b) {
			return DistanceSquared(a->GetWorldPosition(), defendAt)
				< DistanceSquared(b->GetWorldPosition(), defendAt);
		});
		for (auto* n : pool) {
			if ((int)chosen.size() >= count) break;
			if (std::find(chosen.begin(), chosen.end(), n) == chosen.end())
				chosen.push_back(n);
		}
	};
	fillNear(unkC);
	fillNear(atkC);

	for (auto* n : chosen) ToDefense(ps, n, station, defendAt);

	for (auto* n : defC) {
		if (std::find(chosen.begin(), chosen.end(), n) == chosen.end()) {
			ps.duty[n] = Duty::Unknown;
		}
	}
}

void QuotaUnitDirector::AssignAttackQuota(BaseStation* station, int count, BaseStation* rivalStation) {
	if (!station) return;

	if (!rivalStation) {
		const auto total = GetControllableUnitCount(station);
		AssignDefenseQuota(station, total, station->GetWorldPosition());
		return;
	}

	auto& ps = Ensure(station);

	std::vector<NPC*> live = station->GetLiveNpcs();
	GcByLive(ps, live);

	std::vector<NPC*> defC, atkC, unkC;
	SplitByDuty(ps, live, defC, atkC, unkC);

	std::vector<NPC*> chosen; chosen.reserve(count);
	for (auto* n : atkC) {
		if ((int)chosen.size() >= count) break;
		chosen.push_back(n);
	}

	const Vector3 targetPos = rivalStation->GetWorldPosition();
	std::sort(unkC.begin(), unkC.end(), [&](NPC* a, NPC* b) {
		return DistanceSquared(a->GetWorldPosition(), targetPos)
			< DistanceSquared(b->GetWorldPosition(), targetPos);
	});
	for (auto* n : unkC) {
		if ((int)chosen.size() >= count) break;
		chosen.push_back(n);
	}

	for (auto* n : chosen) ToAttack(ps, n, rivalStation);

	for (auto* n : atkC) {
		if (std::find(chosen.begin(), chosen.end(), n) == chosen.end()) {
			ps.duty[n] = Duty::Unknown;
		}
	}
}
