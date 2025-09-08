#include "QuotaUnitDirector.h"
#include "Actor/Boundary/Boundary.h"

// ===== util =====
float QuotaUnitDirector::DistanceSquared(const Vector3& a, const Vector3& b) {
	const float dx = a.x - b.x, dy = a.y - b.y, dz = a.z - b.z;
	return dx * dx + dy * dy + dz * dz;
}

// 決定論的な微小ジッター（並びの同一距離問題を崩す）
float QuotaUnitDirector::HashJitter(const void* p, float amp) {
	if (amp <= 0.0f) return 0.0f;
	auto x = reinterpret_cast<uintptr_t>(p);
	x ^= x >> 33; x *= 0xff51afd7ed558ccdULL;
	x ^= x >> 33; x *= 0xc4ceb9fe1a85ec53ULL;
	x ^= x >> 33;
	const double u01 = (x & 0xFFFFFFFFULL) / double(0xFFFFFFFFULL);
	return float((u01 - 0.5) * 2.0 * amp); // [-amp, +amp]
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

	// 近ければアンカー旋回、遠ければ拠点へ集合
	if (std::sqrt(DistanceSquared(pos, defendAt)) <= cfg_.defendHoldRadius) {
		npc->SetDefendAnchor(defendAt);
		npc->SetTarget(nullptr);
	} else {
		npc->ClearDefendAnchor();
		npc->SetTarget(owner);
	}
	ps.duty[npc] = Duty::Defense;
}

void QuotaUnitDirector::ToAttack(PerStation& ps, NPC* npc, BaseStation* rival) {
	if (!npc) return;
	npc->ClearDefendAnchor();
	npc->SetTarget(rival);
	ps.duty[npc] = Duty::Attack;
}

void QuotaUnitDirector::AssignDefenseQuota(BaseStation* station, int count, const Vector3& defendAt) {
	if (!station) return;
	auto& ps = Ensure(station);
	ps.lastDefendPos = defendAt;

	// ★ AIの要求と絶対下限の大きい方を守る
	const int want = (std::max)(count, cfg_.defenseFloorCount);

	std::vector<NPC*> live = station->GetLiveNpcs();
	GcByLive(ps, live);

	std::vector<NPC*> defC, atkC, unkC;
	SplitByDuty(ps, live, defC, atkC, unkC);

	std::vector<NPC*> chosen; chosen.reserve(want);

	// 既存防衛を優先（ヒステリシス）
	for (auto* n : defC) {
		if ((int)chosen.size() >= want) break;
		chosen.push_back(n);
	}

	// 距離＋ジッターで補充（Unknown→Attack中の順）
	auto scoreNear = [&](NPC* n) { return DistanceSquared(n->GetWorldPosition(), defendAt); };
	auto fillNear = [&](std::vector<NPC*>& pool) {
		std::sort(pool.begin(), pool.end(), [&](NPC* a, NPC* b) {
			return scoreNear(a) + HashJitter(a, cfg_.distanceJitter)
				< scoreNear(b) + HashJitter(b, cfg_.distanceJitter);
		});
		for (auto* n : pool) {
			if ((int)chosen.size() >= want) break;
			if (std::find(chosen.begin(), chosen.end(), n) == chosen.end())
				chosen.push_back(n);
		}
	};
	fillNear(unkC);
	fillNear(atkC);

	for (auto* n : chosen) ToDefense(ps, n, station, defendAt);

	// 選ばれなかった現防衛は Unknown（次で攻撃候補に回る）
	for (auto* n : defC) {
		if (std::find(chosen.begin(), chosen.end(), n) == chosen.end()) {
			ps.duty[n] = Duty::Unknown;
		}
	}
}

void QuotaUnitDirector::AssignAttackQuota(BaseStation* station, int count, BaseStation* rivalStation) {
	if (!station) return;

	// 穴が必要な設定なら、穴が無い時は全員防衛へ
	if (cfg_.requireGateForAttack) {
		const auto& holes = Boundary::GetInstance()->GetHoles();
		if (holes.empty()) {
			const int total = GetControllableUnitCount(station);
			const Vector3 hold = station->GetWorldPosition();
			AssignDefenseQuota(station, total, hold);
			return;
		}
	}

	if (!rivalStation) {
		const int total = GetControllableUnitCount(station);
		AssignDefenseQuota(station, total, station->GetWorldPosition());
		return;
	}

	auto& ps = Ensure(station);
	std::vector<NPC*> live = station->GetLiveNpcs();
	GcByLive(ps, live);

	std::vector<NPC*> defC, atkC, unkC;
	SplitByDuty(ps, live, defC, atkC, unkC);

	// 攻撃上限を適用（0=無制限）
	int cap = count;
	if (cfg_.attackCapAbsolute > 0) cap = std::min(cap, cfg_.attackCapAbsolute);

	// 防衛下限を守るため cap をさらに制限
	const int total = (int)live.size();
	const int keep = std::min(cfg_.defenseFloorCount, total);
	cap = std::min(cap, (std::max)(0, total - keep));

	// 既に攻撃中を優先
	std::vector<NPC*> chosen; chosen.reserve(cap);
	for (auto* n : atkC) {
		if ((int)chosen.size() >= cap) break;
		chosen.push_back(n);
	}

	// 残りは Rival に近い順＋ジッター（防衛中は候補にしない＝防衛維持）
	const Vector3 pivot = rivalStation->GetWorldPosition();
	std::sort(unkC.begin(), unkC.end(), [&](NPC* a, NPC* b) {
		return DistanceSquared(a->GetWorldPosition(), pivot) + HashJitter(a, cfg_.distanceJitter)
			< DistanceSquared(b->GetWorldPosition(), pivot) + HashJitter(b, cfg_.distanceJitter);
	});
	for (auto* n : unkC) {
		if ((int)chosen.size() >= cap) break;
		chosen.push_back(n);
	}

	for (auto* n : chosen) {
		ToAttack(ps, n, rivalStation);
	}

	// 余剰の攻撃中は Unknown（次フェーズで防衛へ回せる）
	for (auto* n : atkC) {
		if (std::find(chosen.begin(), chosen.end(), n) == chosen.end()) {
			ps.duty[n] = Duty::Unknown;
		}
	}
}
