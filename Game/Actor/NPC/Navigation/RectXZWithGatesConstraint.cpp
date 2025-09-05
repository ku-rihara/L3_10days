#include "RectXZWithGatesConstraint.h"
#include "Actor/Boundary/Boundary.h"
#include "Vector3.h"

#include <algorithm>
#include <cmath>

RectXZWithGatesConstraint::RectXZWithGatesConstraint(const IHoleSource* holes,
													 const RectXZ& rect, float eps)
	: holeSrc_(holes), rect_(rect), eps_(eps){}

void RectXZWithGatesConstraint::SetRect(const RectXZ& rect) noexcept{ rect_ = rect; }

bool RectXZWithGatesConstraint::IsPointInsideRectXZ(const Vector3& p) const noexcept{
	return (p.x >= rect_.minX && p.x <= rect_.maxX &&
		p.z >= rect_.minZ && p.z <= rect_.maxZ);
}

/// <summary>
/// Liang–Barsky アルゴリズムで線分と矩形の交差区間を求める。
/// </summary>
bool RectXZWithGatesConstraint::SegmentIntersectRect2D(const Vector3& a, const Vector3& b,
													   float& tEnter, float& tExit,
													   Vector3& pEnter, Vector3& pExit) const noexcept{
	float ax = a.x, az = a.z;
	float bx = b.x, bz = b.z;
	float dx = bx - ax, dz = bz - az;

	float t0 = 0.0f, t1 = 1.0f;

	auto clip = [&](float p, float q)-> bool{
		if (std::abs(p) < 1e-8f){
			if (q < 0.0f) return false;
			return true;
		}
		float r = q / p;
		if (p < 0.0f){
			if (r > t1) return false;
			if (r > t0) t0 = r;
		}
		else{
			if (r < t0) return false;
			if (r < t1) t1 = r;
		}
		return true;
	};

	// 矩形各辺に対してクリッピング
	if (!clip(-dx,ax - rect_.minX)) return false; // 左
	if (!clip(dx,rect_.maxX - ax)) return false; // 右
	if (!clip(-dz,az - rect_.minZ)) return false; // 下
	if (!clip(dz,rect_.maxZ - az)) return false; // 上
	if (t0 > t1) return false;

	// 内部区間 [tEnter, tExit]
	tEnter = std::clamp(t0,0.0f,1.0f);
	tExit = std::clamp(t1,0.0f,1.0f);

	pEnter = {ax + dx * tEnter, a.y + (b.y - a.y) * tEnter, az + dz * tEnter};
	pExit = {ax + dx * tExit, a.y + (b.y - a.y) * tExit, az + dz * tExit};
	return (tEnter <= tExit);
}

/// <summary>
/// 点 (px,pz) と線分 (ax,az)–(bx,bz) の距離を求める。
/// </summary>
float RectXZWithGatesConstraint::DistancePointToSegment2D(float px, float pz,
														  float ax, float az, float bx, float bz) noexcept{
	float vx = bx - ax, vz = bz - az;
	float wx = px - ax, wz = pz - az;
	float c1 = vx * wx + vz * wz;
	if (c1 <= 0.0f) return std::hypot(px - ax,pz - az);
	float c2 = vx * vx + vz * vz;
	if (c2 <= 1e-12f) return std::hypot(px - ax,pz - az);
	float t = c1 / c2;
	if (t >= 1.0f) return std::hypot(px - bx,pz - bz);
	float cx = ax + t * vx, cz = az + t * vz;
	return std::hypot(px - cx,pz - cz);
}

/// <summary>
/// 矩形内部を通る線分がいずれかの穴（円）を通過しているかを判定する。
/// </summary>
bool RectXZWithGatesConstraint::SegmentInsidePassesThroughAnyHole2D(const Vector3& aInside,
																	const Vector3& bInside) const noexcept{
	const auto& holes = holeSrc_->GetHoles();
	for (const auto& h : holes){
		if (h.radius <= 0.0f) continue;
		float d = DistancePointToSegment2D(h.position.x,h.position.z,
										   aInside.x,aInside.z,bInside.x,bInside.z);
		if (d <= h.radius) return true; // 線分が穴円を横切る
	}
	return false;
}

/// <summary>
/// from → to の移動を制約する。
/// 穴を通らずに矩形を横切ろうとした場合、矩形に入る直前で停止させる。
/// </summary>
Vector3 RectXZWithGatesConstraint::FilterMove(const Vector3& from, const Vector3& to) const{
	// 毎回フラグをリセット
	blocked_ = false;

	const bool fromInside = IsPointInsideRectXZ(from);
	const bool toInside = IsPointInsideRectXZ(to);

	// 内→内, 内→外 は制約なし
	if (fromInside && toInside) return to;
	if (fromInside && !toInside) return to;

	// 外→外, 外→内 の場合は矩形との交差を調べる
	float tEnter = 0, tExit = 0;
	Vector3 pEnter, pExit;
	if (!SegmentIntersectRect2D(from,to,tEnter,tExit,pEnter,pExit)){
		return to; // 矩形に触れていない
	}

	// 矩形内部区間が穴を通っていれば通過許可
	if (SegmentInsidePassesThroughAnyHole2D(pEnter,pExit)){ return to; }

	// 穴を通らなかった
	blocked_ = true;

	// 矩形に入る直前で停止させる
	float back = (std::max)(0.0f,tEnter - 1e-4f);
	Vector3 halted = {
				from.x + (to.x - from.x) * back,
				from.y + (to.y - from.y) * back,
				from.z + (to.z - from.z) * back
			};

	// 誤差で食い込まないように押し戻す
	Vector3 step = halted - from;
	float len = step.Length();
	if (len > 1e-6f){
		Vector3 dir = step * (1.0f / len);
		halted = halted - dir * eps_;
	}
	return halted;
}