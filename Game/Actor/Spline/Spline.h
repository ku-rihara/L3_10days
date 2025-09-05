#pragma once

/// std
#include <memory>
#include <vector>
#include <string>

/// engine
#include <Vector3.h>
#include "3d/Line3d.h"
#include "3d/ViewProjection.h"


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

	void DebugDraw(const ViewProjection& _vp);

	size_t GetControlPointCount() const;
	const std::vector<Vector3>& GetControlPoints() const;

private:
	/// ==================================
	/// private : objects
	/// ==================================

	std::vector<Vector3> controlPoints_; ///< コントロールポイント
	size_t controlPointCount_ = 0; ///< コントロールポイント数

	/// ----- debug ----- ///
	std::unique_ptr<Line3D> line3D_ = nullptr;

};

