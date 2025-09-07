#pragma once
#include <vector>
#include <random>
#include "Vector3.h"

class Spline;

class SplineFollower {
public:
    struct RouteParam {
        const Spline* spline = nullptr;
        float speedScale = 1.0f;     // ルートごとの速度倍率
        float weight = 1.0f;         // 初期/切替選択の重み
        float lateralOffset = 0.0f;  // 右(+) / 左(-) の平行ずれ（m）
    };

    struct Output {
        Vector3 desiredDir;  // このフレームの目標向き（正規化）
        float   plannedDist; // このフレームに進みたい距離（baseSpeed×倍率×dt）
        bool    switched;    // 直近でルート切替が発生したか
    };

    // ルート登録（戻り値=登録 index）
    int  AddRoute(const RouteParam& rp);

    // 重み抽選で初期ルート選択
    void SelectInitialRouteWeighted();

    // ルートを明示選択
    void SelectRouteByIndex(int idx);

    // 個体差（スポーン時に呼ぶ）
    void RandomizeIndividual(float speedJitterRatio = 0.1f,
                             float lateralOffsetMax = 3.0f,
                             float switchPeriodMean = 4.0f,
                             float switchProb = 0.35f,
                             uint32_t seed = 0);

    // 回頭制限（deg/s）
    void SetMaxTurnRateDeg(float degPerSec);

    // 現在位置に最寄りスナップして追従開始
    void ResetAt(const Vector3& currentPos);

    // 1フレーム更新：目標向き＆想定距離
    Output Tick(const Vector3& currentPos,
                const Vector3& currentHeading,
                float baseSpeed,
                float dt);

    // 実際に動いた距離で進捗を確定（境界クリップ後の距離を渡す）
    void Advance(float actualMovedDist);

    // 現在ルートの有効性
    bool HasUsableRoute() const noexcept;

    // 現在ルート index（-1=無し）
    int  CurrentRouteIndex() const noexcept { return currentRoute_; }

private:
    struct Route {
        const Spline* spline = nullptr;
        std::vector<Vector3> pts;   // 折れ線頂点（ワールド）
        size_t segIdx = 0;          // 現セグメント開始点
        float  segRemain = 0.0f;    // 現セグメントの終点までの残距離
        float  speedScale = 1.0f;
        float  weight = 1.0f;
        float  lateralOffset = 0.0f;
        bool   usable() const { return spline && pts.size() >= 2; }
    };

    std::vector<Route> routes_;
    int   currentRoute_ = -1;

    // 個体差
    std::mt19937 rng_{ 12345 };
    float indivSpeedScale_ = 1.0f;
    float switchTimer_ = 0.0f;
    float switchPeriod_ = 4.0f;
    float switchProb_ = 0.35f;

    // 姿勢
    float   maxTurnRateDeg_ = 180.0f;
    Vector3 lastDesired_{ 0,0,1 };

private:
    // 内部ヘルパ
    void   RebuildRoutePolyline_(int i);
    void   SnapCursorToNearest_(int i, const Vector3& pos);
    Vector3 RouteDesiredDir_(int i, const Vector3& pos, float lookAheadDist, const Vector3& fallback) const;
    void   AdvanceOnRoute_(int i, float dist);

    static Vector3 AddLateralOffset_(const Vector3& base, const Vector3& dir, float offset);
    static Vector3 RotateToward_(const Vector3& cur, const Vector3& des, float maxAngleRad);

    void   MaybeSwitch_(float dt, const Vector3& pos, bool atSegmentEnd);
    int    DrawWeighted_(float totalW);
};
