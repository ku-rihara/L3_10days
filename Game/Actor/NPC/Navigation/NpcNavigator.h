#pragma once
#include <vector>
#include <limits>
#include "Vector3.h"
#include "../SplineFollower.h"

struct Hole;

// 航法設定
struct NpcNavConfig {
    float speed = 40.0f;
    float minSpeed = 5.0f;
    float maxSpeed = 200.0f;

    // Orbit（ロイター）
    int   orbitClockwise = +1;     // +1:時計回り, -1:反時計
    float orbitRadius = 30.0f;  // 既存の円形ロイター用（フォールバック）
    float orbitRadialGain = 0.5f;
    float orbitTangentBias = 1.0f;
    float orbitAngularSpd = 0.0f;   // スプライン使用時は未使用でもOK

    // 操舵
    float maxTurnRateDeg = 120.0f;

    // 境界・穴
    float passFrac = 0.9f;   // 穴半径×係数で通過可
    float minHoleRadius = 0.1f;
};

class NpcNavigator {
public:
    enum class State { ToTarget, ToHole, Orbit };

    explicit NpcNavigator(const NpcNavConfig& cfg = {}) : cfg_(cfg) {}

    void     Reset(const Vector3& orbitCenter) noexcept;
    void     UpdateSpeed(bool /*boost*/, bool /*brake*/, float /*dt*/);

    // 目標方向（穴/ターゲット/ロイター）計算の補助
    Vector3  DesiredDirToHole(const Vector3& npcPos) const;
    Vector3  DesiredDirToTarget(const Vector3& npcPos, const Vector3& tgtPos) const;
    Vector3  DesiredDirLoiter(const Vector3& npcPos, float t) const;

    int      SelectBestHole(const std::vector<Hole>& holes,
                            const Vector3& npcPos, const Vector3& tgtPos) const;

    void     SteerTowards(const Vector3& desiredDir, float dt, bool isBoosting);
    void     StartOrbit(const Vector3& center) noexcept;
    State    GetState() const noexcept { return state_; }

    Vector3  Tick(float dt,
                  const Vector3& npcPos,
                  const Vector3& tgtPos,
                  const std::vector<Hole>& holes);

    // ===== Orbit用：外からスプラインルートを登録できるAPI =====
    int  AddOrbitRoute(const SplineFollower::RouteParam& rp) { return follower_.AddRoute(rp); }
    void SelectInitialOrbitRoute() { follower_.SelectInitialRouteWeighted(); }
    void RandomizeOrbitIndividual(float speedJitter = 0.10f, float lateralMax = 3.0f,
                                  float switchPeriod = 4.0f, float switchProb = 0.35f, uint32_t seed = 0) {
        follower_.RandomizeIndividual(speedJitter, lateralMax, switchPeriod, switchProb, seed);
    }
    void SetOrbitMaxTurnRate(float degPerSec) { follower_.SetMaxTurnRateDeg(degPerSec); }

private:
    // ヘルパー
    static Vector3 RotateToward(const Vector3& cur, const Vector3& des, float maxAngleRad);

private:
    // 設定
    NpcNavConfig cfg_{};

    // 状態
    State   state_ = State::ToTarget;
    Vector3 heading_{ 0,0,-1 };
    float   speed_ = 40.0f;
    float   bankDeg_ = 0.0f;

    // 穴ターゲット
    int     holeIndex_ = -1;
    Vector3 holePos_{};
    float   holeRadius_ = 0.0f;

    // ロイター（円形フォールバック用）
    Vector3 orbitCenter_{};
    float   orbitAngle_ = 0.0f;

    // スプライン追従（Orbit時のみ使用）
    SplineFollower follower_;
};
