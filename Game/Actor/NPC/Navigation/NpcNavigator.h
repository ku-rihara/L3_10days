#pragma once
#include <vector>
#include "Vector3.h"

struct Hole;

// ===================== 設定 =====================
struct NpcNavConfig {
    // 速度・加減速
    float speed = 35.0f;  // 初期速度
    float minSpeed = 4.0f;
    float maxSpeed = 60.0f;
    float afterburnerSpeedScale = 1.5f;   // ブースト時の最高速倍率
    float thrustAccel = 30.0f;  // 加速[m/s^2]相当
    float brakeAccel = 40.0f;  // 減速[m/s^2]相当
    float dragCoeff = 0.08f;  // 抗力（速度比例）

    // 旋回・見た目
    float maxTurnRateDeg = 180.0f; // [deg/s]（SteerTowardsで使用）
    float turnRateInBoostScale = 1.0f;   // ブースト時の旋回率倍率
    float maxBankDeg = 45.0f;  // バンク最大角
    float bankResponse = 6.0f;   // バンク応答（大きいほど速く追従）

    // 穴・ターゲット・リターゲット
    float minHoleRadius = 8.0f;
    float passFrac = 0.25f;  // 通過判定： holeRadius * passFrac
    float retargetInterval = 0.6f;   // 穴を掴んだ後の再選択クールダウン
    float holeApproachBoostDist = 25.0f;  // これより遠いとToHole中はブースト
    float targetApproachBrakeDist = 18.0f;  // これより近いとToTargetで減速

    // 穴選択のコスト係数
    float angleWeight = 10.0f;  // 現在ヘディングとの角度^2重み
    float offAxisWeight = 2.0f;   // ターゲット軸から外れ角^2重み
    float radiusWeight = 3.0f;   // 半径ボーナス重み

    // ロイター（水平回遊）
    float orbitRadius = 20.0f;
    float orbitAngularSpd = 1.2f;   // [rad/s]
    float orbitRadialGain = 0.6f;   // 半径誤差に対する放射補正ゲイン
    float orbitTangentBias = 1.0f;   // 接線成分の重み
    float idleJitterAmp = 0.5f;   // 微揺らぎの振幅
    float idleJitterFreq = 0.8f;   // 微揺らぎの周波数
    int   orbitClockwise = +1;     // +1:時計回り / -1:反時計回り
};

// ===================== ナビゲータ =====================
class NpcNavigator {
public:
    enum class State { Orbit, ToHole, ToTarget };

    explicit NpcNavigator(const NpcNavConfig& cfg = {}) : cfg_(cfg) {}

    void     Reset(const Vector3& orbitCenter) noexcept;

    // 望ましい移動ベクトル（速度×dt）を返す
    Vector3  Tick(float dt,
                  const Vector3& npcPos,
                  const Vector3& tgtPos,
                  const std::vector<Hole>& holes);

    void     StartOrbit(const Vector3& center) noexcept;
    State    GetState() const noexcept { return state_; }

private:
    // ステア/速度制御
    void     SteerTowards(const Vector3& desiredDir, float dt, bool isBoosting);
    void     UpdateSpeed(bool boost, bool brake, float dt);

    // 指向ベクトル
    Vector3  DesiredDirToHole(const Vector3& npcPos) const;
    Vector3  DesiredDirToTarget(const Vector3& npcPos, const Vector3& tgtPos) const;
    Vector3  DesiredDirLoiter(const Vector3& npcPos, float t) const;

    // 穴選択
    int      SelectBestHole(const std::vector<Hole>& holes,
                            const Vector3& npcPos,
                            const Vector3& tgtPos) const;

private:
    // 設定
    NpcNavConfig cfg_;

    // 状態
    State   state_ = State::Orbit;

    // 穴追跡
    int     holeIndex_ = -1;
    Vector3 holePos_{};
    float   holeRadius_ = 0.0f;
    float   retargetCooldown_ = 0.0f;

    // ヘディング/速度/見た目バンク
    Vector3 heading_{ 0,0,1 };
    float   speed_ = 0.0f;
    float   bankDeg_ = 0.0f;

    // ロイター
    Vector3 orbitCenter_{};
    float   orbitAngle_ = 0.0f;
};
