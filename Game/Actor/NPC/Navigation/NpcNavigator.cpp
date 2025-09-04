#include "NpcNavigator.h"
#include "Actor/Boundary/Boundary.h" // struct Hole { Vector3 position; float radius; ... }
#include <algorithm>
#include <limits>
#include <cmath>

// ===== ヘルパー =====
static inline float Clamp(float v, float lo, float hi) { return (std::max)(lo, (std::min)(v, hi)); }
static inline float Len3(const Vector3& v) { return std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z); }
static inline float Dot(const Vector3& a, const Vector3& b) { return a.x * b.x + a.y * b.y + a.z * b.z; }
static inline Vector3 Normalize3(const Vector3& v, const Vector3& fb = { 0,0,-1 }) {
	float l = Len3(v);
	return (l > 1e-6f) ? (v * (1.0f / l)) : fb;
}

// --- 境界平面(P,N) 取得（Boundary の API に確定）
static inline bool GetBoundaryPlane(Vector3& P, Vector3& N) {
	const auto* bd = Boundary::GetInstance();
	if (!bd) return false;
	bd->GetDividePlane(P, N);
	N = Normalize3(N, { 0,1,0 });  // 念のため正規化
	return true;
}
static inline float Rad(float deg) { return deg * 3.1415926535f / 180.0f; }
static inline bool ShouldCrossBoundary(const Vector3& npcPos, const Vector3& tgtPos) {
	Vector3 P, N;
	if (!GetBoundaryPlane(P, N)) return false; // 境界が無ければ不要
	const float sNpc = Dot(N, npcPos - P);
	const float sTgt = Dot(N, tgtPos - P);
	return (sNpc * sTgt) < 0.0f;
}

// cur から des へ、最大回頭角 maxAngleRad でだけ向きを寄せる
static inline Vector3 RotateToward(const Vector3& cur, const Vector3& des, float maxAngleRad) {
	Vector3 a = Normalize3(cur), b = Normalize3(des);
	float dot = Clamp(a.x * b.x + a.y * b.y + a.z * b.z, -1.0f, 1.0f);
	float theta = std::acos(dot);
	if (theta <= maxAngleRad) return b;
	float t = maxAngleRad / (theta + 1e-8f);
	return Normalize3(a * (1.0f - t) + b * t, b);
}


// 線分 AB が平面(P,N)と交差するなら t∈[0,1] と交点 Q を返す
static inline bool SegmentPlaneHit(const Vector3& A, const Vector3& B,
								   const Vector3& P, const Vector3& N,
								   float& tHit, Vector3& Q) {
	const float da = Dot(N, A - P);
	const float db = Dot(N, B - P);
	if (da * db > 0.0f) return false;                // 同じ側 → 交差なし
	const float denom = (da - db);
	if (std::fabs(denom) < 1e-8f) return false;      // 平行 or 接線
	tHit = da / (da - db);
	if (tHit < 0.0f || tHit > 1.0f) return false;    // 線分内でない
	Q = A + (B - A) * tHit;
	return true;
}

// 交点が通れる穴の中か？（ToHole中は追尾穴を優先）
static inline bool IsPassableAt(const Vector3& Q,
								const std::vector<Hole>& holes,
								float passFrac,
								int preferIndex, float preferRadius) {
	const float preferPass = (std::max)(1.0f, preferRadius * passFrac);
	if (preferIndex >= 0) {
		Vector3 d = holes[preferIndex].position - Q;
		if (Len3(d) <= preferPass) return true;
	}
	for (int i = 0; i < (int)holes.size(); ++i) {
		if (holes[i].radius <= 0.0f) continue;
		const float pass = (std::max)(1.0f, holes[i].radius * passFrac);
		Vector3 d = holes[i].position - Q;
		if (Len3(d) <= pass) return true;
	}
	return false;
}

// ===================================================
// 初期化
void NpcNavigator::Reset(const Vector3& orbitCenter) noexcept {
	state_ = State::ToTarget;      // デフォルトはターゲットへ
	holeIndex_ = -1; holePos_ = {}; holeRadius_ = 0.0f;
	retargetCooldown_ = 0.0f;

	heading_ = { 0,0,-1 };         // モデル前方 -Z
	speed_ = Clamp(cfg_.speed, cfg_.minSpeed, cfg_.maxSpeed);
	bankDeg_ = 0.0f;

	orbitCenter_ = orbitCenter;    // ロイター中心
	orbitAngle_ = 0.0f;
}

// ===================================================
// 速度は一定（cfg_.speed）
void NpcNavigator::UpdateSpeed(bool /*boost*/, bool /*brake*/, float /*dt*/) {
	speed_ = Clamp(cfg_.speed, cfg_.minSpeed, cfg_.maxSpeed);
}

// ===================================================
// 穴の方向ベクトル（3D）
Vector3 NpcNavigator::DesiredDirToHole(const Vector3& npcPos) const {
	return Normalize3(holePos_ - npcPos, { 0,0,-1 });
}

// ターゲットの方向ベクトル（3D）
Vector3 NpcNavigator::DesiredDirToTarget(const Vector3& npcPos, const Vector3& tgtPos) const {
	return Normalize3(tgtPos - npcPos, { 0,0,-1 });
}

// ロイター（水平円運動 + 半径補正）
Vector3 NpcNavigator::DesiredDirLoiter(const Vector3& npcPos, float /*t*/) const {
	Vector3 r = npcPos - orbitCenter_;
	r.y = 0.0f;
	float rl = std::sqrt(r.x * r.x + r.z * r.z);
	Vector3 radial = (rl > 1e-4f) ? (r * (1.0f / rl)) : Vector3{ 1,0,0 };

	// 接線（時計/反時計回り）
	Vector3 tangent = (cfg_.orbitClockwise >= 0)
		? Vector3{ -radial.z, 0,  radial.x }
	: Vector3{ radial.z, 0, -radial.x };

// 目標半径へ寄せる補正
	float err = cfg_.orbitRadius - rl; // 内側(+)→外へ、外側(-)→内へ
	Vector3 radialCorr = radial * (cfg_.orbitRadialGain * err);

	Vector3 des = tangent * cfg_.orbitTangentBias + radialCorr;
	return Normalize3(des, tangent);
}

// ===================================================
// 最も近い有効穴を選ぶ
int NpcNavigator::SelectBestHole(const std::vector<Hole>& holes,
								 const Vector3& npcPos,
								 const Vector3& /*tgtPos*/) const {
	int best = -1;
	float bestD2 = (std::numeric_limits<float>::max)();
	for (int i = 0; i < (int)holes.size(); ++i) {
		const Hole& h = holes[i];
		if (h.radius < cfg_.minHoleRadius) continue;
		Vector3 d = h.position - npcPos;
		float d2 = d.x * d.x + d.y * d.y + d.z * d.z;
		if (d2 < bestD2) { bestD2 = d2; best = i; }
	}
	return best;
}

// ===================================================
void NpcNavigator::SteerTowards(const Vector3& desiredDir, float dt, bool /*isBoosting*/) {
	const float maxTurn = Rad(cfg_.maxTurnRateDeg) * dt;
	heading_ = RotateToward(heading_, desiredDir, maxTurn);
}

// ===================================================
void NpcNavigator::StartOrbit(const Vector3& center) noexcept {
	state_ = State::Orbit;
	orbitCenter_ = center;
}

// ===================================================
Vector3 NpcNavigator::Tick(float dt,
						   const Vector3& npcPos,
						   const Vector3& tgtPos,
						   const std::vector<Hole>& holes) {
	//ここを追加：穴が無い間は常にロイター
	if (holes.empty()) {
		if (state_ != State::Orbit) {
			StartOrbit(npcPos);                 // その場旋回開始
		} else {
			orbitAngle_ += cfg_.orbitAngularSpd * dt;
		}
		Vector3 des = DesiredDirLoiter(npcPos, orbitAngle_);
		SteerTowards(des, dt, false);
		UpdateSpeed(false, false, dt);

		Vector3 delta = heading_ * speed_ * dt;

		// 壁クリップ（穴が無いので絶対に越えない）
		Vector3 P, N;
		if (GetBoundaryPlane(P, N)) {
			float tHit; Vector3 Q;
			const Vector3 nextPos = npcPos + delta;
			if (SegmentPlaneHit(npcPos, nextPos, P, N, tHit, Q)) {
				const float backEps = 0.01f;
				const float tStop = (std::max)(0.0f, tHit - backEps);
				delta = (nextPos - npcPos) * tStop;
				Vector3 tangent = Normalize3(heading_ - N * Dot(heading_, N), heading_);
				heading_ = tangent;
			}
		}
		return delta;
	}

	// ===== 以降は穴がある場合のみ =====
	int  pick = SelectBestHole(holes, npcPos, tgtPos);
	bool hasHole = (pick >= 0);
	bool needCross = ShouldCrossBoundary(npcPos, tgtPos);
	bool useHole = hasHole && needCross;

	if (useHole) {
		if (state_ != State::ToHole || pick != holeIndex_) {
			holeIndex_ = pick;
			holePos_ = holes[pick].position;
			holeRadius_ = holes[pick].radius;
			state_ = State::ToHole;
		}
		Vector3 des = DesiredDirToHole(npcPos);
		SteerTowards(des, dt, false);
		UpdateSpeed(false, false, dt);

		const float d = Len3(holePos_ - npcPos);
		const float passDist = (std::max)(1.0f, holeRadius_ * cfg_.passFrac);
		if (d <= passDist) {
			state_ = State::ToTarget;           // 穴通過後にターゲットへ
			holeIndex_ = -1; holePos_ = {}; holeRadius_ = 0.0f;
		}
	} else if (state_ == State::Orbit) {
		orbitAngle_ += cfg_.orbitAngularSpd * dt;
		Vector3 des = DesiredDirLoiter(npcPos, orbitAngle_);
		SteerTowards(des, dt, false);
		UpdateSpeed(false, false, dt);
	} else {
		// 同じ側なら普通にターゲットへ（防衛帰投など）
		state_ = State::ToTarget;
		Vector3 des = DesiredDirToTarget(npcPos, tgtPos);
		SteerTowards(des, dt, false);
		UpdateSpeed(false, false, dt);
	}

	Vector3 delta = heading_ * speed_ * dt;

	// 境界クリップ（穴以外で越えない）
	Vector3 P, N;
	if (GetBoundaryPlane(P, N)) {
		float tHit; Vector3 Q;
		const Vector3 nextPos = npcPos + delta;
		if (SegmentPlaneHit(npcPos, nextPos, P, N, tHit, Q)) {
			const int   preferIdx = (state_ == State::ToHole) ? holeIndex_ : -1;
			const float preferRadius = (state_ == State::ToHole) ? holeRadius_ : 0.0f;
			const bool passable = IsPassableAt(Q, holes, cfg_.passFrac, preferIdx, preferRadius);
			if (!passable) {
				const float backEps = 0.01f;
				const float tStop = (std::max)(0.0f, tHit - backEps);
				delta = (nextPos - npcPos) * tStop;
				Vector3 tangent = Normalize3(heading_ - N * Dot(heading_, N), heading_);
				heading_ = tangent;
			}
		}
	}
	return delta;
}
