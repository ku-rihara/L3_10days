#include "NpcNavigator.h"
#include "Actor/Boundary/Boundary.h" // GetDividePlane / Holes
#include <algorithm>
#include <cmath>
#include <limits>

// ====== ローカルユーティリティ ======
static inline float Rad(float deg){ return deg * 3.1415926535f / 180.0f; }
static inline float Signf(float x){ return (x >= 0.0f) ? 1.0f : -1.0f; }

static inline Vector3 Normalize3(const Vector3& v, const Vector3& fb = Vector3(0,0,-1)){
    return Vector3::NormalizeOr(v,fb);
}
static inline float Len3(const Vector3& v){ return v.Length(); }
static inline float Dot3(const Vector3& a, const Vector3& b){ return Vector3::Dot(a,b); }

static inline bool GetBoundaryPlane(Vector3& P, Vector3& N){
    const auto* bd = Boundary::GetInstance();
    if (!bd) return false;
    bd->GetDividePlane(P,N);
    N = Normalize3(N,Vector3::ToUp());
    return true;
}
static inline bool ShouldCrossBoundary(const Vector3& npcPos, const Vector3& tgtPos){
    Vector3 P, N;
    if (!GetBoundaryPlane(P,N)) return false;
    const float sNpc = Dot3(N,npcPos - P);
    const float sTgt = Dot3(N,tgtPos - P);
    return (sNpc * sTgt) < 0.0f;
}
static inline bool SegmentPlaneHit(const Vector3& A, const Vector3& B,
                                   const Vector3& P, const Vector3& N,
                                   float& tHit, Vector3& Q){
    const float da = Dot3(N,A - P);
    const float db = Dot3(N,B - P);
    if (da * db > 0.0f) return false;          // 同じ側
    const float denom = (da - db);
    if (std::fabs(denom) < 1e-8f) return false;// ほぼ平行
    tHit = da / (da - db);
    if (tHit < 0.0f || tHit > 1.0f) return false;
    Q = A + (B - A) * tHit;
    return true;
}

// === ToAttack中の“面からの目標クリアランス”と法線バイアス強度 ===
static constexpr float kAttackClearance   = 400.0f; // [m]
static constexpr float kAttackNormalBias  = 0.6f;   // 0..1

// === ToAttack中のリーシュ（敵ステーション中心から離れすぎない） ===
static constexpr float kAttackLeashRadius = 1500.0f; // [m]
static constexpr float kAttackLeashBias   = 0.75f;   // desired への引力の強さ（0..1）
static constexpr float kAttackLeashStart  = 0.90f;   // 半径の何割で引力をかけ始めるか

void NpcNavigator::Reset(const Vector3& orbitCenter) noexcept{
    state_ = State::ToTarget;
    holeIndex_ = -1;
    holePos_ = {};
    holeRadius_ = 0.0f;

    heading_ = Vector3(0,0,-1);
    speed_   = std::clamp(cfg_.speed,cfg_.minSpeed,cfg_.maxSpeed);
    bankDeg_ = 0.0f;

    orbitCenter_ = orbitCenter;
    orbitAngle_  = 0.0f;

    attackCenter_     = {};
    attackGraceTimer_ = 0.0f;

    follower_.SelectInitialRouteWeighted();
    follower_.SetMaxTurnRateDeg(cfg_.maxTurnRateDeg);
    follower_.ResetAt(orbitCenter_);
}

// NPC 側の speed を毎フレーム反映
void NpcNavigator::SetSpeed(float v) noexcept{
    cfg_.speed = v;
    speed_ = std::clamp(cfg_.speed,cfg_.minSpeed,cfg_.maxSpeed);
}

Vector3 NpcNavigator::DesiredDirToHole(const Vector3& npcPos) const{
    return Normalize3(holePos_ - npcPos,Vector3(0,0,-1));
}
Vector3 NpcNavigator::DesiredDirToTarget(const Vector3& npcPos, const Vector3& tgtPos) const{
    return Normalize3(tgtPos - npcPos,Vector3(0,0,-1));
}
Vector3 NpcNavigator::DesiredDirLoiter(const Vector3& npcPos, float /*t*/) const{
    // 円形ロイター（フォールバック）
    Vector3 r = npcPos - orbitCenter_;
    r.y = 0.0f;
    float rl = r.Length();
    Vector3 radial = (rl > 1e-4f) ? (r * (1.0f / rl)) : Vector3(1,0,0);

    Vector3 tangent = (cfg_.orbitClockwise >= 0)
                        ? Vector3(-radial.z,0,radial.x)
                        : Vector3(radial.z,0,-radial.x);

    float err = cfg_.orbitRadius - rl;
    Vector3 radialCorr = radial * (cfg_.orbitRadialGain * err);
    Vector3 des = tangent * cfg_.orbitTangentBias + radialCorr;
    return Normalize3(des,tangent);
}

int NpcNavigator::SelectBestHole(const std::vector<Hole>& holes,
                                 const Vector3& npcPos,
                                 const Vector3& tgtPos) const{
    if (holes.empty()) return -1;

    // NPC→目標の境界交点 Q（無ければ中点）
    Vector3 Q = (npcPos + tgtPos) * 0.5f;
    Vector3 P, N;
    if (GetBoundaryPlane(P,N)){
        float tHit; Vector3 qTmp;
        if (SegmentPlaneHit(npcPos,tgtPos,P,N,tHit,qTmp)){ Q = qTmp; }
    }

    int best = -1;
    float bestSc = (std::numeric_limits<float>::max)();
    const float holeBias = tac_.holeBiasAttack;

    for (int i = 0; i < (int)holes.size(); ++i){
        const auto& h = holes[i];
        if (h.radius < cfg_.minHoleRadius) continue;

        const float d2     = Vector3::Dot(h.position - Q,h.position - Q);
        const float reward = (h.radius * holeBias);
        const float score  = d2 / (reward * reward + 1e-3f);

        if (score < bestSc){ bestSc = score; best = i; }
    }
    return best;
}

Vector3 NpcNavigator::RotateToward(const Vector3& cur, const Vector3& des, float maxAngleRad){
    Vector3 a = Normalize3(cur,Vector3::ToForward());
    Vector3 b = Normalize3(des,Vector3::ToForward());
    float dot = std::clamp(Vector3::Dot(a,b),-1.0f,1.0f);
    float theta = std::acos(dot);
    if (theta <= maxAngleRad) return b;
    float t = maxAngleRad / (theta + 1e-8f);
    Vector3 lerp = a * (1.0f - t) + b * t;
    return Normalize3(lerp,b);
}
void NpcNavigator::SteerTowards(const Vector3& desiredDir, float dt, bool /*isBoosting*/){
    const float maxTurn = Rad(cfg_.maxTurnRateDeg) * dt;
    heading_ = RotateToward(heading_,desiredDir,maxTurn);
}

void NpcNavigator::StartOrbit(const Vector3& center) noexcept{
    state_ = State::Orbit;
    orbitCenter_ = center;
    follower_.ResetAt(center);
}
void NpcNavigator::StartAttack(const Vector3& center) noexcept{
    state_ = State::ToAttack;
    attackCenter_ = center;
}

// ---------------------------
// 戦術ゴール
// ---------------------------
NpcNavigator::TacticalGoal
NpcNavigator::BuildTacticalGoal_(const Vector3& npcPos,
                                 const Vector3& sensedTgt,
                                 const std::vector<Hole>& /*holes*/) const{
    TacticalGoal g{};
    Vector3 P, N;
    const bool hasPlane = GetBoundaryPlane(P,N);

    auto lennz = [](const Vector3& v){ return v.Length() > 1e-3f; };

    switch (role_){
    case Role::AttackBase:{
        const Vector3 primary = side_.enemyBase;
        const float useAltDist = 60.0f;
        const bool hasAlt = lennz(sensedTgt - npcPos) && (sensedTgt - npcPos).Length() <= useAltDist;
        g.target    = hasAlt ? sensedTgt : primary;
        g.needCross = hasPlane ? ShouldCrossBoundary(npcPos,g.target) : false;
        g.holeBias  = tac_.holeBiasAttack;
        break;
    }
    case Role::DefendBase:{
        const Vector3 anchor = side_.allyBase;
        bool intercept = false;
        if (lennz(sensedTgt - npcPos) && hasPlane){
            const float dPlane     = std::fabs(Dot3(N,sensedTgt - P));
            const float nearPlane  = 30.0f;
            const bool tgtSameSide = (Dot3(N,anchor - P) * Dot3(N,sensedTgt - P)) >= 0.0f;
            intercept = (tgtSameSide || dPlane < nearPlane) &&
                        ((sensedTgt - anchor).Length() <= tac_.interceptRange);
        }
        g.target    = intercept ? sensedTgt : anchor;
        g.needCross = false;
        g.holeBias  = tac_.holeBiasDefend;
        break;
    }
    case Role::Patrol:
    default:{
        const Vector3 anchor = side_.allyBase;
        g.target    = lennz(sensedTgt - npcPos) ? sensedTgt : anchor;
        g.needCross = hasPlane ? ShouldCrossBoundary(npcPos,g.target) : false;
        g.holeBias  = 0.8f;
        break;
    }}
    return g;
}

bool NpcNavigator::TryEnterToHole_(const std::vector<Hole>& holes,
                                   const Vector3& npcPos,
                                   const Vector3& tgtPos){
    const int pick = SelectBestHole(holes,npcPos,tgtPos);
    if (pick < 0) return false;

    holeIndex_  = pick;
    holePos_    = holes[pick].position;
    holeRadius_ = holes[pick].radius;
    state_      = State::ToHole;

    // 少しの間は ToHole を維持（穴チラつき対策）
    toHoleHoldTimer_ = 0.35f;
    return true;
}

// ---------------------------
// Tick
// ---------------------------
Vector3 NpcNavigator::Tick(float dt,
                           const Vector3& npcPos,
                           const Vector3& tgtPos,
                           const std::vector<Hole>& holes){
    const TacticalGoal goal = BuildTacticalGoal_(npcPos, tgtPos, holes);

    if (state_ != State::ToHole){
        if (goal.needCross){
            if (TryEnterToHole_(holes, npcPos, goal.target)){
                return {0,0,0}; // 遷移のみ
            }
        }

        // === Orbit / ToAttack 共通：スプライン追従 ===
        if (state_ == State::Orbit || state_ == State::ToAttack){
            if (!follower_.HasUsableRoute()) return {0,0,0};

            const auto out = follower_.Tick(npcPos, Normalize3(heading_), speed_, dt);
            Vector3 desired = out.desiredDir;

            // ToAttack中は中心（敵ステーション）にリーシュ
            if (state_ == State::ToAttack) {
                // ステーションが動く場合に備え、毎フレーム同期
                attackCenter_ = side_.enemyBase;

                // --- [ToAttack クリアランス補正] ---
                Vector3 P, N;
                if (GetBoundaryPlane(P, N)){
                    const float enemySide = Dot3(N, side_.enemyBase - P);
                    const float sideSign  = Signf(enemySide != 0.0f ? enemySide : 1.0f);

                    const float s       = Dot3(N, npcPos - P);                 // 現在の符号付き距離
                    const float targetS = sideSign * kAttackClearance;         // 目標距離
                    const float err     = targetS - s;
                    const float normErr = std::clamp(err / (kAttackClearance + 1e-6f), -1.0f, 1.0f);

                    const Vector3 normalBias = N * (normErr * kAttackNormalBias);
                    desired = Normalize3(desired + normalBias, desired);
                }
                // --- [/ToAttack クリアランス補正] ---

                // --- [リーシュ：desired への中心引力] ---
                const Vector3 toCenter = attackCenter_ - npcPos;
                const float dist = toCenter.Length();
                if (dist > (kAttackLeashRadius * kAttackLeashStart)) {
                    const float excess = dist - (kAttackLeashRadius * kAttackLeashStart);
                    const float span   = kAttackLeashRadius * (1.0f - kAttackLeashStart) + 1e-6f;
                    const float w      = std::clamp(excess / span, 0.0f, 1.0f); // 0..1
                    const Vector3 pull = Normalize3(toCenter) * (w * kAttackLeashBias);
                    desired = Normalize3(desired + pull, desired);
                }
                // --- [/リーシュ] ---
            }

            SteerTowards(desired, dt, false);

            // 実移動距離 = speed * dt
            speed_ = std::clamp(cfg_.speed, cfg_.minSpeed, cfg_.maxSpeed);
            const float intendedLen = speed_ * dt;

            // 操舵後の heading_ を使用
            Vector3 moveDir = Normalize3(heading_, Vector3::ToForward());
            Vector3 delta   = moveDir * intendedLen;

            // クリップ条件：ToAttack はグレース中は無効
            const bool doClip = (state_ == State::Orbit) ||
                                (state_ == State::ToAttack && attackGraceTimer_ <= 0.0f);

            if (doClip){
                Vector3 P, N;
                if (GetBoundaryPlane(P, N)){
                    float tHit; Vector3 Q;
                    const Vector3 nextPos = npcPos + delta;
                    if (SegmentPlaneHit(npcPos, nextPos, P, N, tHit, Q)){
                        const float tClamped  = std::clamp(tHit, 0.0f, 1.0f);
                        const float lenToWall = tClamped * intendedLen;
                        const float lenRemain = (std::max)(0.0f, intendedLen - lenToWall);

                        const float pushBack  = 0.05f;
                        const float lenToUse  = (std::max)(0.0f, lenToWall - pushBack);

                        auto safeTangent = [&](const Vector3& prefer)->Vector3{
                            Vector3 t = prefer - N * Dot3(prefer, N);
                            float L = t.Length();
                            if (L > 1e-4f) return t * (1.0f / L);
                            const Vector3 up = (std::fabs(Dot3(N, Vector3::ToUp())) > 0.95f)
                                                ? Vector3::ToRight() : Vector3::ToUp();
                            t = Vector3::Cross(up, N);
                            L = t.Length();
                            return (L > 1e-4f) ? (t * (1.0f / L)) : Vector3::ToRight();
                        };

                        const Vector3 tangent     = safeTangent(moveDir);
                        const Vector3 deltaToWall = moveDir * lenToUse;
                        const Vector3 deltaSlide  = tangent * lenRemain;

                        delta    = deltaToWall + deltaSlide; // スライド
                        heading_ = tangent;                  // 次フレも接線へ
                    }
                }
            }

            // --- [リーシュ：外側拡大の抑止（ハード）] ---
            if (state_ == State::ToAttack) {
                const Vector3 r = Normalize3(npcPos - attackCenter_, Vector3::ToRight()); // 外向き単位ベクトル
                const float distNow = (npcPos - attackCenter_).Length();
                if (distNow > kAttackLeashRadius) {
                    const float outward = Dot3(delta, r);
                    if (outward > 0.0f) {
                        // 外向き成分を削除（接線/内向きのみ通す）
                        delta -= r * outward;
                    }
                }
            }
            // --- [/リーシュ：外側拡大の抑止] ---

            // グレース消費
            if (attackGraceTimer_ > 0.0f){
                attackGraceTimer_ = (std::max)(0.0f, attackGraceTimer_ - dt);
            }

            follower_.Advance(delta.Length());
            return delta;
        }

        // === 初期/ToTarget等は Orbit を開始 ===
        if (state_ != State::Orbit && state_ != State::ToAttack){
            const Vector3 orbitCenter =
                (role_ == Role::DefendBase || role_ == Role::Patrol) ? side_.allyBase : side_.allyBase;
            if (role_ == Role::DefendBase)      cfg_.orbitRadius = tac_.defendOrbitRadius;
            else if (role_ == Role::Patrol)     cfg_.orbitRadius = tac_.patrolOrbitRadius;
            else /*AttackBase*/                  cfg_.orbitRadius = tac_.patrolOrbitRadius;
            StartOrbit(orbitCenter);
        }
        orbitAngle_ += cfg_.orbitAngularSpd * dt;
        return {0,0,0};
    }

    // --------------------------------
    // ToHole 中：穴に向かって進む
    // --------------------------------
    toHoleHoldTimer_ = (std::max)(0.0f, toHoleHoldTimer_ - dt);

    // 開閉に追随：毎フレーム選び直す
    const int pick = SelectBestHole(holes, npcPos, goal.target);
    if (pick < 0){
        if (toHoleHoldTimer_ <= 0.0f){
            state_ = State::Orbit;
            follower_.ResetAt(npcPos);
        }
        return {0,0,0};
    }
    if (pick != holeIndex_){
        holeIndex_  = pick;
        holePos_    = holes[pick].position;
        holeRadius_ = holes[pick].radius;
    }

    // 穴方向へ操舵
    const Vector3 des = Normalize3(holePos_ - npcPos, Vector3(0,0,-1));
    SteerTowards(des, dt, false);

    speed_ = std::clamp(cfg_.speed, cfg_.minSpeed, cfg_.maxSpeed);
    Vector3 delta = heading_ * (speed_ * dt);

    // 通過判定 → ToAttack（攻撃用スプラインへ）
    const float d        = Len3(holePos_ - npcPos);
    const float passDist = (std::max)(1.0f, holeRadius_ * cfg_.passFrac);
    if (d <= passDist){
        // クリア
        holeIndex_  = -1;
        holePos_    = {};
        holeRadius_ = 0.0f;

        // --- 押し出し：敵拠点側へ小さくキック ---
        Vector3 crossPos = npcPos + delta; // 今フレームの着地点
        Vector3 P,N;
        if (GetBoundaryPlane(P,N)){
            const float enemySide = Dot3(N, side_.enemyBase - P);
            const float sideSign  = Signf(enemySide != 0.0f ? enemySide : 1.0f);

            // 面近傍なら押し出す
            const float s = Dot3(N, crossPos - P);
            const float epsBand = 0.25f;
            if (std::fabs(s) < epsBand || Signf(s) != sideSign){
                crossPos += N * (cfg_.exitKickDist * sideSign);
            }

            // ヘディングにも法線バイアス
            Vector3 tang  = Normalize3(heading_ - N * Dot3(heading_,N), heading_);
            Vector3 biased = Normalize3(tang + N * (cfg_.exitNormalBoost * sideSign), tang);
            heading_ = biased;
        }

        // 攻撃用ルートへ
        StartAttack(side_.enemyBase);
        follower_.ResetAt(crossPos);

        // グレースでクリップ無効
        attackGraceTimer_ = 0.35f;

        // 押し出しを反映した移動量で返す
        return (crossPos - npcPos);
    }
    return delta;
}