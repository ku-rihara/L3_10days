#pragma once

/// std
#include <memory>
#include <vector>
#include <string>

/// engine
#include <Vector3.h>
#include "3d/Line3d.h"

/// //////////////////////////////////
/// spline曲線の制御
/// //////////////////////////////////
class Spline {
public:
	/// ==================================
	/// public : methods
	/// ==================================

	Spline();
	~Spline();

	void Load(const std::string& _filepath);

	void DebugDraw(const class ViewProjection& _vp);

	// ---- accessor ---------------------------------------------------------
	size_t GetControlPointCount() const;
	const std::vector<Vector3>& GetControlPoints() const;
	std::vector<Vector3>& GetControlPointsMutable();

	// ---- 補間API ----------------------------------------------------------
	/// 正規化パラメータ u ∈ [0,1] から位置を取得
	Vector3 Evaluate(float u) const;

private:
	/// ==================================
	/// private : objects
	/// ==================================
	std::vector<Vector3> controlPoints_; ///< コントロールポイント
	size_t controlPointCount_ = 0;       ///< コントロールポイント数

	/// ----- debug ----- ///
	std::unique_ptr<Line3D> line3D_ = nullptr;

	/// ==================================
	/// private : helpers
	/// ==================================
	/// Catmull-Rom 補間
	Vector3 CatmullRom(const Vector3& p0, const Vector3& p1,
					   const Vector3& p2, const Vector3& p3,
					   float t) const;
};