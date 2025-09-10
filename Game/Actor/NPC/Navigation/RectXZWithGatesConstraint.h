#pragma once

#include "MoveConstraint.h"
#include "Actor/Boundary/RectXZ.h"

/// <summary>
/// 矩形とrayの衝突をとる(境界内の穴を考慮
/// </summary>
class RectXZWithGatesConstraint : public IMoveConstraint{
public:
	/// <param name="holes">穴一覧の供給元 (Boundary など)</param>
	/// <param name="rect">通行制約とする矩形</param>
	/// <param name="eps">押し戻し距離（誤差防止用の微小値）</param>
	RectXZWithGatesConstraint(const IHoleSource* holes, const RectXZ& rect, float eps = 0.01f);

	/// <summary>
	/// 矩形を再設定する
	/// </summary>
	void SetRect(const RectXZ& rect) noexcept;

	/// <param name="from">移動開始点</param>
	/// <param name="to">移動希望点</param>
	/// <returns>制約を考慮した移動後の座標</returns>
	Vector3 FilterMove(const Vector3& from, const Vector3& to) const override;
	Vector3 FilterMoveReflect(const Vector3& from, const Vector3& to) const override;

	/// <summary>
	/// true の場合、境界で止められてる
	/// </summary>
	bool WasBlocked() const noexcept{ return blocked_; }

private:
	/// <summary>点が矩形内部にあるかを判定</summary>
	bool IsPointInsideRectXZ(const Vector3& p) const noexcept;

	/// <summary>
	/// レイ飛ばして矩形と衝突判定
	/// </summary>
	/// <returns>矩形と交差していれば true</returns>
	bool SegmentIntersectRect2D(const Vector3& a, const Vector3& b,
								float& tEnter, float& tExit,
								Vector3& pEnter, Vector3& pExit) const noexcept;

	/// <summary>
	/// 矩形内部の線分がいずれかの穴（円）を通過しているか判定
	/// </summary>
	bool SegmentInsidePassesThroughAnyHole2D(const Vector3& aInside, const Vector3& bInside) const noexcept;

	/// <summary>
	/// 2D 線分と点の距離を求める
	/// </summary>
	static float DistancePointToSegment2D(float px, float pz,
										  float ax, float az, float bx, float bz) noexcept;

private:
	const IHoleSource* holeSrc_ = nullptr; ///< 穴一覧の供給元
	RectXZ rect_{-1500.f * 5.0f, 1500.f * 5.0f, -1500.f * 5.0f, 1500.f * 5.0f }; ///< 矩形領域
	float eps_ = 0.01f; ///< 押し戻し距離（誤差防止）
	float restitution_      = 0.2f; // 0=吸収, 1=完全反射
	float tangentFriction_  = 0.9f; // 1=減衰なし, 0=完全停止
	mutable bool blocked_ = false; ///< 直前の FilterMove でブロックされたかどうか
};