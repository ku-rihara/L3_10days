#include "SplineFollower.h"
#include "Actor/Spline/Spline.h"
#include <algorithm>
#include <limits>
#include <cmath>

static inline float Clamp1(float v, float lo, float hi) { return (v < lo) ? lo : ((v > hi) ? hi : v); }
static inline float Rad(float deg) { return deg * 3.1415926535f / 180.0f; }
static inline float Dist3(const Vector3& a, const Vector3& b) { return (a - b).Length(); }

int SplineFollower::AddRoute(const RouteParam& rp) {
    Route r;
    r.spline = rp.spline;
    r.speedScale = (rp.speedScale > 0.01f) ? rp.speedScale : 1.0f;
    r.weight = (rp.weight > 0.0f) ? rp.weight : 1.0f;
    r.lateralOffset = rp.lateralOffset;
    routes_.push_back(std::move(r));
    int idx = (int)routes_.size() - 1;
    RebuildRoutePolyline_(idx);
    return idx;
}

void SplineFollower::RebuildRoutePolyline_(int i) {
    if (i < 0 || i >= (int)routes_.size()) return;
    auto& r = routes_[i];
    r.pts.clear();
    if (!r.spline) return;
    const auto& cps = r.spline->GetControlPoints();
    r.pts.insert(r.pts.end(), cps.begin(), cps.end());
    r.segIdx = 0;
    r.segRemain = 0.0f;
}

void SplineFollower::SelectInitialRouteWeighted() {
    if (routes_.empty()) { currentRoute_ = -1; return; }
    float total = 0.f;
    for (auto& r : routes_) total += (r.weight > 0.f ? r.weight : 0.f);
    currentRoute_ = DrawWeighted_((total > 0.f) ? total : (float)routes_.size());
}

void SplineFollower::SelectRouteByIndex(int idx) {
    if (idx >= 0 && idx < (int)routes_.size() && routes_[idx].usable()) currentRoute_ = idx;
}

int SplineFollower::DrawWeighted_(float totalW) {
    if (routes_.empty()) return -1;
    if (totalW <= 0.f) return 0;
    std::uniform_real_distribution<float> uni(0.f, totalW);
    float x = uni(rng_);
    float run = 0.f;
    for (int i = 0;i < (int)routes_.size();++i) {
        float w = (routes_[i].weight > 0.f ? routes_[i].weight : 0.f);
        run += (w > 0.f ? w : 1.f);
        if (run >= x) return i;
    }
    return (int)routes_.size() - 1;
}

void SplineFollower::RandomizeIndividual(float speedJitterRatio,
                                         float lateralOffsetMax,
                                         float switchPeriodMean,
                                         float switchProb,
                                         uint32_t seed) {
    if (seed != 0) rng_.seed(seed);
    std::uniform_real_distribution<float> U(-1.f, 1.f);
    std::uniform_real_distribution<float> U01(0.f, 1.f);

    indivSpeedScale_ = 1.f + Clamp1(speedJitterRatio, 0.f, 1.f) * U(rng_);
    switchPeriod_ =(std::max)(0.5f, switchPeriodMean * (0.75f + 0.5f * U01(rng_)));
    switchProb_ = Clamp1(switchProb, 0.f, 1.f);

    for (auto& r : routes_) {
        r.lateralOffset += lateralOffsetMax * U(rng_);
    }
}

void SplineFollower::SetMaxTurnRateDeg(float degPerSec) {
    maxTurnRateDeg_ = (degPerSec > 0.f ? degPerSec : 180.f);
}

bool SplineFollower::HasUsableRoute() const noexcept {
    return (currentRoute_ >= 0 && currentRoute_ < (int)routes_.size() && routes_[currentRoute_].usable());
}

void SplineFollower::SnapCursorToNearest_(int i, const Vector3& pos) {
    auto& r = routes_[i];
    if (!r.usable()) { r.segIdx = 0; r.segRemain = 0.0f; return; }

    const size_t n = r.pts.size();
    size_t bestI = 0;
    float  bestD2 = (std::numeric_limits<float>::max)();
    float  bestRemain = 0.0f;

    for (size_t k = 0; k < n; ++k) {
        const Vector3 A = r.pts[k];     
        const Vector3 B = r.pts[(k + 1) % n];
        Vector3 AB = B - A;
        float ab2 = Vector3::Dot(AB, AB);
        if (ab2 < 1e-8f) continue;
        Vector3 AP = pos - A;
        float t = Vector3::Dot(AP, AB) / ab2;
        t = Clamp1(t, 0.f, 1.f);
        Vector3 Q = A + AB * t;
        Vector3 PQ = pos - Q;
        float d2 = Vector3::Dot(PQ, PQ);
        if (d2 < bestD2) {
            bestD2 = d2;
            bestI = (int)k;
            bestRemain = Dist3(Q, B);   // Q→B の残距離
        }
    }
    r.segIdx = bestI;
    r.segRemain = bestRemain;
}

Vector3 SplineFollower::AddLateralOffset_(const Vector3& base, const Vector3& dir, float offset) {
    const Vector3 up = Vector3::ToUp(); // (0,1,0)
    Vector3 right = Vector3::Cross(up, Vector3::NormalizeOr(dir, Vector3::ToRight()));
    return base + right * offset;
}

Vector3 SplineFollower::RouteDesiredDir_(int i, const Vector3& pos, float lookAheadDist, const Vector3& fallback) const {
    const auto& r = routes_[i];
    if (!r.usable()) return fallback;

    const size_t n = r.pts.size();
    size_t seg = r.segIdx;
    float  remain = r.segRemain;

    const Vector3 A0 = r.pts[seg];
    const Vector3 B0 = r.pts[(seg + 1) % n];
    const float segLen0 = Dist3(A0, B0);
    Vector3 abDir = (segLen0 > 1e-6f) ? Vector3::NormalizeOr(B0 - A0, Vector3::ToRight())
        : Vector3::ToRight();

// 現在点（セグメント上）
    Vector3 cur = A0 + abDir * (segLen0 - remain);

    // 目標点：lookAheadDist 先（複数セグメント跨ぎ可）
    float   need = lookAheadDist;
    Vector3 p = cur;
    size_t  s = seg;
    float   rem = remain;

    while (need > rem + 1e-6f) {
        need -= rem;
        s = (s + 1) % n;
        const Vector3 A = r.pts[s];
        const Vector3 B = r.pts[(s + 1) % n];
        float segLen = Dist3(A, B);
        if (segLen < 1e-6f) { rem = 0.f; break; }
        p = A;
        rem = segLen;
    }
    if (rem > 1e-6f && need > 0.f) {
        const Vector3 A = r.pts[s];
        const Vector3 B = r.pts[(s + 1) % n];
        Vector3 d = Vector3::NormalizeOr(B - A, abDir);
        p = p + d *(std::min)(need, rem);
        abDir = d; // 直近方向を更新
    }

    // 平行レーンの横ずれ
    p = AddLateralOffset_(p, abDir, r.lateralOffset);

    return Vector3::NormalizeOr(p - pos, abDir);
}

Vector3 SplineFollower::RotateToward_(const Vector3& cur, const Vector3& des, float maxAngleRad) {
    Vector3 a = Vector3::NormalizeOr(cur, Vector3::ToForward());
    Vector3 b = Vector3::NormalizeOr(des, Vector3::ToForward());
    float dot = std::clamp(Vector3::Dot(a, b), -1.0f, 1.0f);
    float theta = std::acos(dot);
    if (theta <= maxAngleRad) return b;
    float t = maxAngleRad / (theta + 1e-8f);
    Vector3 lerp = a * (1.0f - t) + b * t;
    return Vector3::NormalizeOr(lerp, b);
}

void SplineFollower::ResetAt(const Vector3& currentPos) {
    if (routes_.empty()) { currentRoute_ = -1; return; }
    if (currentRoute_ < 0 || !routes_[currentRoute_].usable()) {
        SelectInitialRouteWeighted();
    }
    if (currentRoute_ >= 0) SnapCursorToNearest_(currentRoute_, currentPos);
    switchTimer_ = 0.0f;
    lastDesired_ = Vector3::ToForward();
}

SplineFollower::Output SplineFollower::Tick(const Vector3& currentPos,
                                            const Vector3& currentHeading,
                                            float baseSpeed,
                                            float dt) {
    Output o{};
    if (!HasUsableRoute()) {
        o.desiredDir = currentHeading;
        o.plannedDist =(std::max)(0.0f, baseSpeed * dt);
        o.switched = false;
        return o;
    }

    const auto& r = routes_[currentRoute_];
    const float planned =(std::max)(0.0f, baseSpeed * indivSpeedScale_ * r.speedScale * dt);

    // 少し先を見越す
    const float lookAhead = planned;
    Vector3 desired = RouteDesiredDir_(currentRoute_, currentPos, lookAhead, currentHeading);

    // 回頭制限
    const float maxTurn = Rad(maxTurnRateDeg_) * dt;
    Vector3 turned = RotateToward_(currentHeading, desired, maxTurn);

    o.desiredDir = turned;
    o.plannedDist = planned;
    o.switched = false;

    // 端トリガ（簡易）
    bool atEnd = (routes_[currentRoute_].segRemain <= 1e-6f);
    MaybeSwitch_(dt, currentPos, atEnd);

    lastDesired_ = turned;
    return o;
}

void SplineFollower::Advance(float actualMovedDist) {
    if (!HasUsableRoute() || actualMovedDist <= 0.0f) return;
    AdvanceOnRoute_(currentRoute_, actualMovedDist);
}

void SplineFollower::AdvanceOnRoute_(int i, float dist) {
    auto& r = routes_[i];
    if (!r.usable()) return;
    const size_t n = r.pts.size();
    float rem = r.segRemain;
    while (dist > 0.0f) {
        if (rem <= 1e-6f) {
            r.segIdx = (r.segIdx + 1) % n;
            const Vector3 A = r.pts[r.segIdx];
            const Vector3 B = r.pts[(r.segIdx + 1) % n];
            rem = Dist3(A, B);
            if (rem < 1e-6f) { r.segRemain = 0.0f; return; }
        }
        float step =(std::min)(dist, rem);
        rem -= step;
        dist -= step;
    }
    r.segRemain = rem;
}

void SplineFollower::MaybeSwitch_(float dt, const Vector3& pos, bool atSegmentEnd) {
    if (routes_.size() <= 1) return;

    switchTimer_ += dt;
    const bool timeTrigger = (switchTimer_ >= switchPeriod_);
    const bool endTrigger = atSegmentEnd;

    if (!(timeTrigger || endTrigger)) return;

    std::uniform_real_distribution<float> U01(0.f, 1.f);
    if (U01(rng_) <= switchProb_) {
        float total = 0.f;
        for (auto& r : routes_) total += (r.weight > 0.f ? r.weight : 0.f);

        int pick;
        do {
            pick = DrawWeighted_((total > 0.f) ? total : (float)routes_.size());
        } while (pick == currentRoute_ && routes_.size() > 1);

        currentRoute_ = pick;
        SnapCursorToNearest_(currentRoute_, pos);
        switchTimer_ = 0.0f;
    } else {
        if (timeTrigger) switchTimer_ = 0.0f;
    }
}
