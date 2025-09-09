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

    // ---- 補間API ----------------------------------------------------------
    /// 正規化パラメータ u ∈ [0,1] から位置を取得
    Vector3 Evaluate(float u) const;

    /// ★追加: Route などから呼べる「サンプル関数」
    Vector3 Sample(float u) const { return Evaluate(u); }

private:
    std::vector<Vector3> controlPoints_; ///< コントロールポイント
    std::unique_ptr<Line3D> line3D_ = nullptr;

    /// Catmull-Rom 補間
    Vector3 CatmullRom(const Vector3& p0, const Vector3& p1,
                       const Vector3& p2, const Vector3& p3,
                       float t) const;
};
