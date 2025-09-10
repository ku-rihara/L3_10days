#include "NpcNavigator.h"
#include "Actor/Boundary/Boundary.h" // GetDividePlane / Holes
#include <algorithm>
#include <cmath>
#include <limits>

// ====== ローカルユーティリティ ======
static inline float Rad(float deg){ return deg * 3.1415926535f / 180.0f; }

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
    if (da * db > 0.0f) return false;
    const float denom = (da - db);
    if (std::fabs(denom) < 1e-8f) return false;
    tHit = da / (da - db);
    if (tHit < 0.0f || tHit > 1.0f) return false;
    Q = A + (B - A) * tHit;
    return true;
}

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

// NPC 側の speed を毎フレーム反映（GUI変更も即反映させる）
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
    // 円形ロイター（スプライン未設定時のフォールバック）
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

    // 境界面と NPC→目標 の交点 Q を求める（なければ中点でフォールバック）
    Vector3 Q = (npcPos + tgtPos) * 0.5f;
    Vector3 P, N;
    if (GetBoundaryPlane(P,N)){
        float tHit;
        Vector3 qTmp;
        if (SegmentPlaneHit(npcPos,tgtPos,P,N,tHit,qTmp)){ Q = qTmp; }
    }

    int best = -1;
    float bestSc = (std::numeric_limits<float>::max)();

    // 役割バイアス（必要なら tac_ から出し分けてもOK。ここでは攻撃寄り）
    const float holeBias = tac_.holeBiasAttack;

    for (int i = 0; i < (int)holes.size(); ++i){
        const auto& h = holes[i];
        if (h.radius < cfg_.minHoleRadius) continue;

        // スコア：交点 Q からの距離を半径で割引（大きい穴ほど有利）
        const float d2 = Vector3::Dot(h.position - Q,h.position - Q);
        const float reward = (h.radius * holeBias);
        const float score = d2 / (reward * reward + 1e-3f);

        if (score < bestSc){
            bestSc = score;
            best = i;
        }
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
    follower_.ResetAt(center); // スプラインに即スナップ（「線に乗る」）
}

void NpcNavigator::StartAttack(const Vector3& center) noexcept{
    state_ = State::ToAttack;
    attackCenter_ = center;
}

// ---------------------------
// 役割に応じた戦術ゴール計算
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
    case Role::AttackBase:
        {
            const Vector3 primary = side_.enemyBase;
            const float useAltDist = 60.0f;
            const bool hasAlt = lennz(sensedTgt - npcPos) && (sensedTgt - npcPos).Length() <= useAltDist;
            g.target   = hasAlt ? sensedTgt : primary;
            g.needCross = hasPlane ? ShouldCrossBoundary(npcPos,g.target) : false;
            g.holeBias = tac_.holeBiasAttack;
            break;
        }
    case Role::DefendBase:
        {
            const Vector3 anchor = side_.allyBase;
            bool intercept = false;
            if (lennz(sensedTgt - npcPos) && hasPlane){
                const float dPlane = std::fabs(Dot3(N,sensedTgt - P));
                const float nearPlane = 30.0f;
                const bool tgtSameSide = (Dot3(N,anchor - P) * Dot3(N,sensedTgt - P)) >= 0.0f;
                intercept = (tgtSameSide || dPlane < nearPlane) &&
                        ((sensedTgt - anchor).Length() <= tac_.interceptRange);
            }
            g.target   = intercept ? sensedTgt : anchor;
            g.needCross = false;
            g.holeBias = tac_.holeBiasDefend;
            break;
        }
    case Role::Patrol:
    default:
        {
            const Vector3 anchor = side_.allyBase;
            g.target   = lennz(sensedTgt - npcPos) ? sensedTgt : anchor;
            g.needCross = hasPlane ? ShouldCrossBoundary(npcPos,g.target) : false;
            g.holeBias = 0.8f;
            break;
        }
    }
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

    // 少しの間は ToHole を維持（ターゲット揺れ/穴チラつき対策）
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
                // 遷移だけ済ませ、このフレームは移動しない
                return {0,0,0};
            }
            // 穴が無ければ従来どおりスプライン追従を継続
        }

        // === Orbit / ToAttack 共通：スプライン追従 ===
        if (state_ == State::Orbit || state_ == State::ToAttack){
            if (!follower_.HasUsableRoute()){ return {0,0,0}; }

            const auto out = follower_.Tick(npcPos, Normalize3(heading_), speed_, dt);
            const Vector3 desired = out.desiredDir;

            // 回頭は常に操舵を通す
            SteerTowards(desired, dt, false);

            // 実移動距離 = speed * dt を厳守
            speed_ = std::clamp(cfg_.speed, cfg_.minSpeed, cfg_.maxSpeed);
            const float intendedLen = speed_ * dt;

            // 進行方向（操舵後の heading_ を使用）
            Vector3 moveDir = Normalize3(heading_, Vector3::ToForward());
            Vector3 delta   = moveDir * intendedLen;

            // クリップ条件
            const bool doClip = (state_ == State::Orbit) ||
                                (state_ == State::ToAttack && attackGraceTimer_ <= 0.0f);

            if (doClip){
                Vector3 P, N;
                if (GetBoundaryPlane(P, N)){
                    float tHit; Vector3 Q;
                    const Vector3 nextPos = npcPos + delta;
                    if (SegmentPlaneHit(npcPos, nextPos, P, N, tHit, Q)){
                        // 壁直前までの距離と残り距離に分解
                        const float tClamped  = std::clamp(tHit, 0.0f, 1.0f);
                        const float lenToWall = tClamped * intendedLen;
                        const float lenRemain = (std::max)(0.0f, intendedLen - lenToWall);

                        // めり込み防止の押し戻し
                        const float pushBack = 0.05f;
                        const float lenToUse = (std::max)(0.0f, lenToWall - pushBack);

                        // 安全な接線方向
                        auto safeTangent = [&](const Vector3& prefer)->Vector3{
                            Vector3 t = prefer - N * Dot3(prefer, N);
                            float L = t.Length();
                            if (L > 1e-4f) return t * (1.0f / L);
                            // prefer がほぼ法線方向なら別基底で作る
                            const Vector3 up = (std::fabs(Dot3(N, Vector3::ToUp())) > 0.95f)
                                               ? Vector3::ToRight() : Vector3::ToUp();
                            t = Vector3::Cross(up, N);
                            L = t.Length();
                            return (L > 1e-4f) ? (t * (1.0f / L)) : Vector3::ToRight();
                        };

                        const Vector3 tangent     = safeTangent(moveDir);
                        const Vector3 deltaToWall = moveDir * lenToUse;
                        const Vector3 deltaSlide  = tangent * lenRemain;

                        delta    = deltaToWall + deltaSlide; // 止めずにスライド
                        heading_ = tangent;                  // 次フレームの向きも接線へ
                    }
                }
            }

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

        // ルート追従時は未使用だが保持
        orbitAngle_ += cfg_.orbitAngularSpd * dt;
        return {0,0,0};
    }

    // --------------------------------
    // ToHole 中：穴に向かって進む
    // --------------------------------
    toHoleHoldTimer_ = (std::max)(0.0f, toHoleHoldTimer_ - dt);

    // 毎フレーム、目標方向の交点Qに近い穴を再選択（開閉に追随）
    const int pick = SelectBestHole(holes, npcPos, goal.target);
    if (pick < 0){
        // 穴が消えた & ホールドも切れたら Orbit 復帰
        if (toHoleHoldTimer_ <= 0.0f){
            state_ = State::Orbit;
            follower_.ResetAt(npcPos); // その場で線へ復帰
        }
        return {0,0,0};
    }
    if (pick != holeIndex_){
        holeIndex_  = pick;
        holePos_    = holes[pick].position;
        holeRadius_ = holes[pick].radius;
    }

    // 穴方向へ操舵して進む
    const Vector3 des = Normalize3(holePos_ - npcPos, Vector3(0,0,-1));
    SteerTowards(des, dt, false);

    speed_ = std::clamp(cfg_.speed, cfg_.minSpeed, cfg_.maxSpeed);
    Vector3 delta = heading_ * (speed_ * dt);

    // 通過判定 → ToAttack（攻撃用スプラインへ）
    const float d        = Len3(holePos_ - npcPos);
    const float passDist = (std::max)(1.0f, holeRadius_ * cfg_.passFrac);
    if (d <= passDist){
        holeIndex_  = -1;
        holePos_    = {};
        holeRadius_ = 0.0f;

        // 攻撃中心＝敵拠点へ
        StartAttack(side_.enemyBase);
        attackGraceTimer_ = 0.25f; // 押し込み猶予

        return delta; // 穴を抜けた慣性で前進
    }
    return delta;
}