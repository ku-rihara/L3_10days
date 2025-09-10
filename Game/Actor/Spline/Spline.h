#pragma once

// std
#include <memory>
#include <vector>
#include <string>

// engine
#include "Vector3.h"
#include "3d/Line3D.h"

/// //////////////////////////////////
/// spline曲線の制御
/// //////////////////////////////////
class Spline {
public:
    Spline();
    ~Spline();

    void Load(const std::string& filepath);
    void DebugDraw(const class ViewProjection& vp);

    // ---- accessor ---------------------------------------------------------
    size_t GetControlPointCount() const;
    const std::vector<Vector3>& GetControlPoints() const;
    std::vector<Vector3>& GetControlPointsMutable();

    // 弧長テーブル関連
    void  BuildArcLengthTable(int samples = 200);
    bool  IsArcLengthBuilt() const noexcept { return arcBuilt_; }
    float GetTotalLength() const noexcept   { return totalLength_; }
    float GetParamByDistance(float dist) const; // dist を [0,L) に wrap して u を返す

    // ---- 補間API ----------------------------------------------------------
    /// 正規化パラメータ u ∈ [0,1] から位置を取得
    Vector3 Evaluate(float u) const;

    Vector3 Sample(float u) const { return Evaluate(u); }

    Vector3 Tangent(float u) const;

private:
    // Catmull-Rom 補間
    Vector3 CatmullRom(const Vector3& p0, const Vector3& p1,
                       const Vector3& p2, const Vector3& p3,
                       float t) const;

    // 弧長テーブルを無効化（Load 時など）
    void InvalidateArcLength_() noexcept;

private:
    std::vector<Vector3> controlPoints_; ///< コントロールポイント
    std::unique_ptr<Line3D> line3D_ = nullptr;

    // 距離テーブル
    std::vector<float> arcLength_;
    float totalLength_ = 0.0f;
    bool  arcBuilt_    = false;
};