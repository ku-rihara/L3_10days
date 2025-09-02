#pragma once

#include "MoveConstraint.h"

#include "Actor/Boundary/RectXZ.h"

class RectXZWithGatesConstraint : public IMoveConstraint {
public:
	RectXZWithGatesConstraint(const IHoleSource* holes, const RectXZ& rect, float eps = 0.01f);

	void   SetRect(const RectXZ& rect) noexcept;
	Vector3 FilterMove(const Vector3& from, const Vector3& to) const override;

private:
	bool   IsPointInsideRectXZ(const Vector3& p) const noexcept;
	bool   SegmentIntersectRect2D(const Vector3& a, const Vector3& b,
								  float& tEnter, float& tExit,
								  Vector3& pEnter, Vector3& pExit) const noexcept;
	bool   SegmentInsidePassesThroughAnyHole2D(const Vector3& aInside, const Vector3& bInside) const noexcept;
	static float DistancePointToSegment2D(float px, float pz,
										  float ax, float az, float bx, float bz) noexcept;

private:
	const IHoleSource* holeSrc_ = nullptr;
	RectXZ rect_{ -1500.f, 1500.f, -1500.f, 1500.f };
	float eps_ = 0.01f; // 押し戻し/数値誤差
};