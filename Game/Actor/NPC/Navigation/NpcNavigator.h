#pragma once
#include <vector>
#include <limits>
#include "Vector3.h"
#include "../SplineFollower.h"

struct Hole;
class Route;

// 航法設定
struct NpcNavConfig {
    float speed = 40.0f;
    float minSpeed = 5.0f;
    float maxSpeed = 200.0f;

    // Orbit（ロイター）
    int   orbitClockwise = +1;     // +1:時計回り, -1:反時計
    float orbitRadius = 30.0f;     // 既存の円形ロイター用（フォールバック）
    float orbitRadialGain = 0.5f;
    float orbitTangentBias = 1.0f;
    float orbitAngularSpd = 0.0f;  // スプライン使用時は未使用でもOK

    // 操舵
    float maxTurnRateDeg = 120.0f;

    // 境界・穴
    float passFrac = 0.9f;         // 穴半径×係数で通過可
    float minHoleRadius = 0.1f;
};

class NpcNavigator {
public:
    enum class State { ToTarget, ToHole, Orbit };

    // 役割（上位のStation/Directorの意思決定を受ける）
    enum class Role : uint8_t {
        AttackBase,   // 敵拠点へ攻勢（越境前提、穴を積極活用）
        DefendBase,   // 自陣側で防衛/哨戒（原則越境しない）
        Patrol,       // 汎用哨戒（状況次第で越境もあり）
    };

    // 陣営情報（拠点座標）
    struct StationSide {
        Vector3 allyBase{};   // 自陣の基準点（防衛アンカー）
        Vector3 enemyBase{};  // 敵拠点
    };

    // 役割別の戦術パラメータ
    struct TacticsConfig {
        float defendOrbitRadius = 40.0f; // 防衛時のロイター半径
        float patrolOrbitRadius = 55.0f; // 哨戒時のロイター半径
        float interceptRange = 120.0f;   // 迎撃を開始する距離（自陣基準）
        float regroupDist = 160.0f;      // 自陣から離れすぎたら戻る距離
        float holeBiasAttack = 1.0f;     // 攻撃時の穴選好（>=1で強め）
        float holeBiasDefend = 0.6f;     // 防衛時の穴選好（<1で弱め）
    };

public:
    explicit NpcNavigator(const NpcNavConfig& cfg = {}) : cfg_(cfg) {}

    // 役割/陣営の設定（毎フレームでもOK）
    void     SetRole(Role r) noexcept { role_ = r; }
    void     SetStationSide(const StationSide& s) noexcept { side_ = s; }
    void     SetTacticsConfig(const TacticsConfig& t) noexcept { tac_ = t; }

    void     Reset(const Vector3& orbitCenter) noexcept;

    // NPC 側の speed を毎フレーム反映（超重要）
    void     SetSpeed(float v) noexcept;

    // 目標方向（穴/ターゲット/ロイター）計算の補助
    Vector3  DesiredDirToHole(const Vector3& npcPos) const;
    Vector3  DesiredDirToTarget(const Vector3& npcPos, const Vector3& tgtPos) const;
    Vector3  DesiredDirLoiter(const Vector3& npcPos, float t) const;

    int      SelectBestHole(const std::vector<Hole>& holes,
                            const Vector3& npcPos, const Vector3& tgtPos) const;

    void     SteerTowards(const Vector3& desiredDir, float dt, bool isBoosting);
    void     StartOrbit(const Vector3& center) noexcept;
    State    GetState() const noexcept { return state_; }

    // メイン駆動
    Vector3  Tick(float dt,
                  const Vector3& npcPos,
                  const Vector3& tgtPos,
                  const std::vector<Hole>& holes);

    // ===== Orbit用：外からスプラインルートを登録できるAPI =====
    void BindOrbitRoute(Route* route) { follower_.BindRoute(route); }
    void SelectInitialOrbitRoute() { follower_.SelectInitialRouteWeighted(); }
    void RandomizeOrbitIndividual(float speedJitter = 0.10f, float lateralMax = 3.0f,
                                  float switchPeriod = 4.0f, float switchProb = 0.35f, uint32_t seed = 0) {
        follower_.RandomizeIndividual(speedJitter, lateralMax, switchPeriod, switchProb, seed);
    }
    void SetOrbitMaxTurnRate(float degPerSec) { follower_.SetMaxTurnRateDeg(degPerSec); }

    // Orbit復帰/開始時に、現在位置の最近点へ即スナップ（線に乗り直す）
    void ResetFollowerAt(const Vector3& currentPos) { follower_.ResetAt(currentPos); }

private:
    // 戦術ゴール（役割により目標/越境/穴選好を決める）
    struct TacticalGoal {
        Vector3 target{};          // 実際に向かう最終目標（敵拠点/自陣アンカー/迎撃対象）
        bool    needCross = false;  // 境界を越すべきか
        float   holeBias = 1.0f;    // 穴選好強度
    };
    TacticalGoal BuildTacticalGoal_(const Vector3& npcPos,
                                    const Vector3& sensedTgt,
                                    const std::vector<Hole>& holes) const;

    // ヘルパー
    static Vector3 RotateToward(const Vector3& cur, const Vector3& des, float maxAngleRad);

private:
    // 設定
    NpcNavConfig  cfg_{};
    TacticsConfig tac_{};

    // 役割/陣営
    Role         role_ = Role::Patrol;
    StationSide  side_{};

    // 状態
    State   state_ = State::Orbit;
    Vector3 heading_{ 0,0,-1 };
    float   speed_ = 40.0f;  // ← NPC から SetSpeed() で毎フレーム更新する
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
