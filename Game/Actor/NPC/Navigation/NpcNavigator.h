#pragma once
#include <vector>
#include <limits>
#include "Vector3.h"
#include "../SplineFollower.h"

struct Hole;
class Route;

// 航法設定
struct NpcNavConfig{
    float speed = 40.0f;
    float minSpeed = 5.0f;
    float maxSpeed = 200.0f;

    // Orbit（ロイター）
    int   orbitClockwise   = +1;   // +1:時計回り, -1:反時計
    float orbitRadius      = 30.0f; // フォールバック用
    float orbitRadialGain  = 0.5f;
    float orbitTangentBias = 1.0f;
    float orbitAngularSpd  = 0.0f;

    // 操舵
    float maxTurnRateDeg   = 120.0f;

    // 境界・穴
    float passFrac      = 0.9f;    // 穴半径×係数で通過可
    float minHoleRadius = 0.1f;

    // ★ 穴抜け後の押し出し/向きバイアス
    float exitKickDist     = 2.0f; // 境界法線方向への押し出し距離
    float exitNormalBoost  = 0.6f; // 押し出し方向にヘディングへ足す比率
};

class NpcNavigator{
public:
    enum class State{ ToTarget, ToHole, Orbit, ToAttack };

    // 役割
    enum class Role : uint8_t{
        AttackBase,
        DefendBase,
        Patrol,
    };

    // 陣営情報（拠点座標）
    struct StationSide{
        Vector3 allyBase{};
        Vector3 enemyBase{};
    };

    // 役割別パラメータ
    struct TacticsConfig{
        float defendOrbitRadius = 40.0f;
        float patrolOrbitRadius = 55.0f;
        float interceptRange    = 120.0f;
        float regroupDist       = 160.0f;
        float holeBiasAttack    = 1.0f;
        float holeBiasDefend    = 0.6f;
    };

public:
    explicit NpcNavigator(const NpcNavConfig& cfg = {}) : cfg_(cfg){}

    void SetRole(Role r) noexcept{ role_ = r; }
    Role GetRole() const noexcept { return role_; }
    void SetStationSide(const StationSide& s) noexcept{ side_ = s; }
    void SetTacticsConfig(const TacticsConfig& t) noexcept{ tac_ = t; }

    void Reset(const Vector3& orbitCenter) noexcept;

    // NPC 側の speed を毎フレーム反映
    void SetSpeed(float v) noexcept;

    // 目標方向
    Vector3 DesiredDirToHole(const Vector3& npcPos) const;
    Vector3 DesiredDirToTarget(const Vector3& npcPos, const Vector3& tgtPos) const;
    Vector3 DesiredDirLoiter(const Vector3& npcPos, float t) const;

    int  SelectBestHole(const std::vector<Hole>& holes,
                        const Vector3& npcPos, const Vector3& tgtPos) const;

    void SteerTowards(const Vector3& desiredDir, float dt, bool isBoosting);
    void StartOrbit(const Vector3& center) noexcept;

    // ★ 攻撃フェーズ
    void StartAttack(const Vector3& center) noexcept;     // ToAttackへ遷移
    void SetAttackGrace(float sec) noexcept { attackGraceTimer_ = sec; }
    bool InAttackGrace() const noexcept { return attackGraceTimer_ > 0.0f; }

    State GetState() const noexcept{ return state_; }

    // メイン駆動
    Vector3 Tick(float dt,
                 const Vector3& npcPos,
                 const Vector3& tgtPos,
                 const std::vector<Hole>& holes);

    // ===== スプライン：外からバインドできるAPI =====
    void BindOrbitRoute(Route* route){ follower_.BindRoute(route); }
    void SelectInitialOrbitRoute(){ follower_.SelectInitialRouteWeighted(); }

    void RandomizeOrbitIndividual(float speedJitter = 0.10f, float lateralMax = 3.0f,
                                  float switchPeriod = 4.0f, float switchProb = 0.35f, uint32_t seed = 0){
        follower_.RandomizeIndividual(speedJitter,lateralMax,switchPeriod,switchProb,seed);
    }
    void SetOrbitMaxTurnRate(float degPerSec){ follower_.SetMaxTurnRateDeg(degPerSec); }

    void ResetFollowerAt(const Vector3& currentPos){ follower_.ResetAt(currentPos); }

private:
    struct TacticalGoal{
        Vector3 target{};
        bool    needCross = false;
        float   holeBias  = 1.0f;
    };

    bool TryEnterToHole_(const std::vector<Hole>& holes,
                         const Vector3& npcPos,
                         const Vector3& tgtPos);
    TacticalGoal BuildTacticalGoal_(const Vector3& npcPos,
                                    const Vector3& sensedTgt,
                                    const std::vector<Hole>& holes) const;

    static Vector3 RotateToward(const Vector3& cur, const Vector3& des, float maxAngleRad);

private:
    // 設定
    NpcNavConfig   cfg_{};
    TacticsConfig  tac_{};

    // 役割/陣営
    Role         role_ = Role::Patrol;
    StationSide  side_{};

    // 状態
    State   state_   = State::Orbit;
    Vector3 heading_{0, 0, -1};
    float   speed_   = 40.0f;
    float   bankDeg_ = 0.0f;

    // 穴ターゲット
    int     holeIndex_   = -1;
    Vector3 holePos_{};
    float   holeRadius_  = 0.0f;
    float   toHoleHoldTimer_ = 0.0f;

    // 攻撃フェーズ
    Vector3 attackCenter_{};
    float   attackGraceTimer_ = 0.0f; // 境界押し込みグレース

    // ロイター（円形フォールバック用）
    Vector3 orbitCenter_{};
    float   orbitAngle_ = 0.0f;

    // スプライン追従（Orbit/ToAttack で使用）
    SplineFollower follower_;
};