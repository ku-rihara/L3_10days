#include "NpcNavigator.h"
#include "Actor/Boundary/Boundary.h" // GetDividePlane / Holes
#include <algorithm>
#include <cmath>

// ====== ローカルユーティリティ ======
static inline float Clamp1(float v, float lo, float hi){ return (v < lo) ? lo : ((v > hi) ? hi : v); }
static inline float Rad(float deg){ return deg * 3.1415926535f / 180.0f; }

static inline Vector3 Normalize3(const Vector3& v, const Vector3& fb = Vector3(0, 0, -1)){
	return Vector3::NormalizeOr(v, fb);
}
static inline float   Len3(const Vector3& v){ return v.Length(); }
static inline float   Dot3(const Vector3& a, const Vector3& b){ return Vector3::Dot(a, b); }

static inline bool GetBoundaryPlane(Vector3& P, Vector3& N){
	const auto* bd = Boundary::GetInstance();
	if (!bd) return false;
	bd->GetDividePlane(P, N);
	N = Normalize3(N, Vector3::ToUp());
	return true;
}
static inline bool ShouldCrossBoundary(const Vector3& npcPos, const Vector3& tgtPos){
	Vector3 P, N; if (!GetBoundaryPlane(P, N)) return false;
	const float sNpc = Dot3(N, npcPos - P);
	const float sTgt = Dot3(N, tgtPos - P);
	return (sNpc * sTgt) < 0.0f;
}
static inline bool SegmentPlaneHit(const Vector3& A, const Vector3& B,
								   const Vector3& P, const Vector3& N,
								   float& tHit, Vector3& Q){
	const float da = Dot3(N, A - P);
	const float db = Dot3(N, B - P);
	if (da * db > 0.0f) return false;
	const float denom = (da - db);
	if (std::fabs(denom) < 1e-8f) return false;
	tHit = da / (da - db);
	if (tHit < 0.0f || tHit > 1.0f) return false;
	Q = A + (B - A) * tHit;
	return true;
}
static inline bool IsPassableAt(const Vector3& Q,
								const std::vector<Hole>& holes,
								float passFrac,
								int preferIndex, float preferRadius){
	const float preferPass = ( std::max ) (1.0f, preferRadius * passFrac);
	if (preferIndex >= 0){
		const Vector3 d = holes[preferIndex].position - Q;
		if (Len3(d) <= preferPass) return true;
	}
	for (int i = 0; i < ( int ) holes.size(); ++i){
		if (holes[i].radius <= 0.0f) continue;
		const float pass = ( std::max ) (1.0f, holes[i].radius * passFrac);
		const Vector3 d = holes[i].position - Q;
		if (Len3(d) <= pass) return true;
	}
	return false;
}

void NpcNavigator::Reset(const Vector3& orbitCenter) noexcept{
	state_ = State::ToTarget;
	holeIndex_ = -1; holePos_ = {}; holeRadius_ = 0.0f;

	heading_ = Vector3(0, 0, -1);
	speed_ = Clamp1(cfg_.speed, cfg_.minSpeed, cfg_.maxSpeed);
	bankDeg_ = 0.0f;

	orbitCenter_ = orbitCenter;
	orbitAngle_ = 0.0f;

	follower_.SelectInitialRouteWeighted();
	follower_.SetMaxTurnRateDeg(cfg_.maxTurnRateDeg);
	follower_.ResetAt(orbitCenter_);
}

void NpcNavigator::UpdateSpeed(bool /*boost*/, bool /*brake*/, float /*dt*/){
	speed_ = Clamp1(cfg_.speed, cfg_.minSpeed, cfg_.maxSpeed);
}

Vector3 NpcNavigator::DesiredDirToHole(const Vector3& npcPos) const{
	return Normalize3(holePos_ - npcPos, Vector3(0, 0, -1));
}

Vector3 NpcNavigator::DesiredDirToTarget(const Vector3& npcPos, const Vector3& tgtPos) const{
	return Normalize3(tgtPos - npcPos, Vector3(0, 0, -1));
}

Vector3 NpcNavigator::DesiredDirLoiter(const Vector3& npcPos, float /*t*/) const{
	// 円形ロイター（スプライン未設定時のフォールバック）
	Vector3 r = npcPos - orbitCenter_; r.y = 0.0f;
	float rl = r.Length();
	Vector3 radial = (rl > 1e-4f) ? (r * (1.0f / rl)) : Vector3(1, 0, 0);

	Vector3 tangent = (cfg_.orbitClockwise >= 0)
		? Vector3(-radial.z, 0, radial.x)
		: Vector3(radial.z, 0, -radial.x);

	float err = cfg_.orbitRadius - rl;
	Vector3 radialCorr = radial * (cfg_.orbitRadialGain * err);
	Vector3 des = tangent * cfg_.orbitTangentBias + radialCorr;
	return Normalize3(des, tangent);
}

int NpcNavigator::SelectBestHole(const std::vector<Hole>& holes,
								 const Vector3& npcPos,
								 const Vector3& /*tgtPos*/) const{
	int best = -1;
	float bestD2 = (std::numeric_limits<float>::max)();
	for (int i = 0; i < ( int ) holes.size(); ++i){
		const auto& h = holes[i];
		if (h.radius < cfg_.minHoleRadius) continue;
		Vector3 d = h.position - npcPos;
		float d2 = Vector3::Dot(d, d);
		if (d2 < bestD2){ bestD2 = d2; best = i; }
	}
	return best;
}

Vector3 NpcNavigator::RotateToward(const Vector3& cur, const Vector3& des, float maxAngleRad){
	Vector3 a = Normalize3(cur, Vector3::ToForward());
	Vector3 b = Normalize3(des, Vector3::ToForward());
	float dot = std::clamp(Vector3::Dot(a, b), -1.0f, 1.0f);
	float theta = std::acos(dot);
	if (theta <= maxAngleRad) return b;
	float t = maxAngleRad / (theta + 1e-8f);
	Vector3 lerp = a * (1.0f - t) + b * t;
	return Normalize3(lerp, b);
}

void NpcNavigator::SteerTowards(const Vector3& desiredDir, float dt, bool /*isBoosting*/){
	const float maxTurn = Rad(cfg_.maxTurnRateDeg) * dt;
	heading_ = RotateToward(heading_, desiredDir, maxTurn);
}

void NpcNavigator::StartOrbit(const Vector3& center) noexcept{
	state_ = State::Orbit;
	orbitCenter_ = center;

	// ルート追従は Orbit のみで使用
	follower_.ResetAt(center);
}

// ---------------------------
// 役割に応じた戦術ゴール計算
// ---------------------------
NpcNavigator::TacticalGoal
NpcNavigator::BuildTacticalGoal_(const Vector3& npcPos,
								 const Vector3& sensedTgt,
								 const std::vector<Hole>& /*holes*/) const{
	TacticalGoal g {};
	Vector3 P, N; const bool hasPlane = GetBoundaryPlane(P, N);

	auto lennz = [] (const Vector3& v){ return v.Length() > 1e-3f; };

	switch (role_){
		case Role::AttackBase:
		{
			// 基本は敵拠点へ。近距離に敵機が見えていればそちらを短期優先
			const Vector3 primary = side_.enemyBase;
			const float useAltDist = 60.0f;
			const bool hasAlt = lennz(sensedTgt - npcPos) && (sensedTgt - npcPos).Length() <= useAltDist;
			g.target = hasAlt ? sensedTgt : primary;
			g.needCross = hasPlane ? ShouldCrossBoundary(npcPos, g.target) : false;
			g.holeBias = tac_.holeBiasAttack;
			break;
		}
		case Role::DefendBase:
		{
			// 自陣側で哨戒/防衛。敵が自陣側/境界近なら迎撃
			const Vector3 anchor = side_.allyBase;
			bool intercept = false;
			if (lennz(sensedTgt - npcPos) && hasPlane){
				const float dPlane = std::fabs(Dot3(N, sensedTgt - P));
				const float nearPlane = 30.0f; // 定数
				const bool tgtSameSide = (Dot3(N, anchor - P) * Dot3(N, sensedTgt - P)) >= 0.0f;
				intercept = (tgtSameSide || dPlane < nearPlane) &&
					((sensedTgt - anchor).Length() <= tac_.interceptRange);
			}
			if (intercept){
				g.target = sensedTgt;
			} else{
				// 離れすぎたらアンカーへ回帰、近ければその周辺でロイター
				g.target = ((npcPos - anchor).Length() > tac_.regroupDist) ? anchor : anchor;
			}
			g.needCross = false;
			g.holeBias = tac_.holeBiasDefend;
			break;
		}
		case Role::Patrol:
		default:
		{
			// 中庸：敵がいれば追う、いなければ自陣アンカー周辺でロイター
			const Vector3 anchor = side_.allyBase;
			g.target = lennz(sensedTgt - npcPos) ? sensedTgt : anchor;
			g.needCross = hasPlane ? ShouldCrossBoundary(npcPos, g.target) : false;
			g.holeBias = 0.8f;
			break;
		}
	}
	return g;
}

// ---------------------------
// Tick（役割駆動版）
// ---------------------------
/////////////////////////////////////////////////////////////////////////////////////////
//      Tick（役割駆動版）※スプラインは Orbit 時のみ使用
/////////////////////////////////////////////////////////////////////////////////////////
Vector3 NpcNavigator::Tick(float dt,
						   const Vector3& npcPos,
						   const Vector3& tgtPos,
						   const std::vector<Hole>& holes){
	const TacticalGoal goal = BuildTacticalGoal_(npcPos, tgtPos, holes);

	// ==== 穴が無いケース：基本は Orbit ====
	if (holes.empty()){
		const Vector3 orbitCenterByRole =
			(role_ == Role::DefendBase || role_ == Role::Patrol) ? side_.allyBase : npcPos;

		if (state_ != State::Orbit){
			if (role_ == Role::DefendBase){      cfg_.orbitRadius = tac_.defendOrbitRadius; }
			else if (role_ == Role::Patrol){     cfg_.orbitRadius = tac_.patrolOrbitRadius; }
			StartOrbit(orbitCenterByRole);
		} else {
			orbitAngle_ += cfg_.orbitAngularSpd * dt;
		}

		Vector3 desired;
		if (state_ == State::Orbit && follower_.HasUsableRoute()){
			const auto out = follower_.Tick(npcPos, Normalize3(heading_), speed_, dt);
			desired = out.desiredDir;
		} else {
			desired = DesiredDirLoiter(npcPos, orbitAngle_);
		}

		SteerTowards(desired, dt, false);
		UpdateSpeed(false, false, dt);

		Vector3 delta = heading_ * speed_ * dt;

		// 境界クリップ（穴が無いので越えない）
		Vector3 P, N;
		if (GetBoundaryPlane(P, N)){
			float tHit; Vector3 Q;
			const Vector3 nextPos = npcPos + delta;
			if (SegmentPlaneHit(npcPos, nextPos, P, N, tHit, Q)){
				const float backEps = 0.01f;
				const float tStop = (std::max)(0.0f, tHit - backEps);
				delta = (nextPos - npcPos) * tStop;
				Vector3 tangent = Normalize3(heading_ - N * Dot3(heading_, N), heading_);
				heading_ = tangent;
			}
		}

		//  Advance は Orbit 時のみ
		if (state_ == State::Orbit && follower_.HasUsableRoute()){
			follower_.Advance(Len3(delta));
		}
		return delta;
	}

	// ==== 穴があるケース ====
	const int  pick = SelectBestHole(holes, npcPos, goal.target);
	const bool hasHole = (pick >= 0);
	const bool useHole = hasHole && goal.needCross;

	if (useHole){
		if (state_ != State::ToHole || pick != holeIndex_){
			holeIndex_  = pick;
			holePos_    = holes[pick].position;
			holeRadius_ = holes[pick].radius;
			state_ = State::ToHole;
		}

		heading_ = Vector3::Normalize(holePos_ - npcPos);

		const float d = Len3(holePos_ - npcPos);
		const float passDist = (std::max)(1.0f, holeRadius_ * cfg_.passFrac);
		if (d <= passDist){
			state_ = State::ToTarget;
			holeIndex_ = -1; holePos_ = {}; holeRadius_ = 0.0f;
		}
	} else{
		if (role_ == Role::DefendBase || role_ == Role::Patrol){
			const Vector3 orbitCenterByRole = side_.allyBase;
			if (state_ != State::Orbit){
				if (role_ == Role::DefendBase){ cfg_.orbitRadius = tac_.defendOrbitRadius; }
				else{                           cfg_.orbitRadius = tac_.patrolOrbitRadius;  }
				StartOrbit(orbitCenterByRole);
			} else{
				orbitAngle_ += cfg_.orbitAngularSpd * dt;
			}

			Vector3 des;
			if (state_ == State::Orbit && follower_.HasUsableRoute()){
				const auto out = follower_.Tick(npcPos, Normalize3(heading_), speed_, dt);
				des = out.desiredDir;
			} else{
				des = DesiredDirLoiter(npcPos, orbitAngle_);
			}
			SteerTowards(des, dt, false);
			UpdateSpeed(false, false, dt);
		} else{
			state_ = State::ToTarget;
			Vector3 des = DesiredDirToTarget(npcPos, goal.target);
			SteerTowards(des, dt, false);
			UpdateSpeed(false, false, dt);
		}
	}

	Vector3 delta = heading_ * speed_ * dt;

	if (state_ == State::Orbit && follower_.HasUsableRoute()){
		follower_.Advance(Len3(delta));
	}
	return delta;
}