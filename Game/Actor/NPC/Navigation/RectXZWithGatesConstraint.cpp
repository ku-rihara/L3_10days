#include "RectXZWithGatesConstraint.h"
#include "Actor/Boundary/Boundary.h"
#include "Vector3.h"

#include <algorithm>
#include <cmath>

RectXZWithGatesConstraint::RectXZWithGatesConstraint(const IHoleSource* holes,
													 const RectXZ& rect, float eps)
	: holeSrc_(holes), rect_(rect), eps_(eps) {}

void RectXZWithGatesConstraint::SetRect(const RectXZ& rect) noexcept { rect_ = rect; }

bool RectXZWithGatesConstraint::IsPointInsideRectXZ(const Vector3& p) const noexcept {
	return (p.x >= rect_.minX && p.x <= rect_.maxX &&
			p.z >= rect_.minZ && p.z <= rect_.maxZ);
}

// Liang–Barsky で線分×矩形（XZ）交差区間 [tEnter, tExit] を求める
bool RectXZWithGatesConstraint::SegmentIntersectRect2D(const Vector3& a, const Vector3& b,
													   float& tEnter, float& tExit,
													   Vector3& pEnter, Vector3& pExit) const noexcept {
	float ax = a.x, az = a.z;
	float bx = b.x, bz = b.z;
	float dx = bx - ax, dz = bz - az;

	float t0 = 0.0f, t1 = 1.0f;

	auto clip = [&](float p, float q)->bool {
		if (std::abs(p) < 1e-8f) { if (q < 0.0f) return false; return true; }
		float r = q / p;
		if (p < 0.0f) { if (r > t1) return false; if (r > t0) t0 = r; } else { if (r < t0) return false; if (r < t1) t1 = r; }
		return true;
	};

	if (!clip(-dx, ax - rect_.minX)) return false; // 左
	if (!clip(dx, rect_.maxX - ax)) return false; // 右
	if (!clip(-dz, az - rect_.minZ)) return false; // 下
	if (!clip(dz, rect_.maxZ - az)) return false; // 上
	if (t0 > t1) return false;

	tEnter = std::clamp(t0, 0.0f, 1.0f);
	tExit = std::clamp(t1, 0.0f, 1.0f);

	pEnter = { ax + dx * tEnter, a.y + (b.y - a.y) * tEnter, az + dz * tEnter };
	pExit = { ax + dx * tExit,  a.y + (b.y - a.y) * tExit,  az + dz * tExit };
	return (tEnter <= tExit);
}

float RectXZWithGatesConstraint::DistancePointToSegment2D(float px, float pz,
														  float ax, float az, float bx, float bz) noexcept {
	float vx = bx - ax, vz = bz - az;
	float wx = px - ax, wz = pz - az;
	float c1 = vx * wx + vz * wz;
	if (c1 <= 0.0f) return std::hypot(px - ax, pz - az);
	float c2 = vx * vx + vz * vz;
	if (c2 <= 1e-12f) return std::hypot(px - ax, pz - az);
	float t = c1 / c2;
	if (t >= 1.0f) return std::hypot(px - bx, pz - bz);
	float cx = ax + t * vx, cz = az + t * vz;
	return std::hypot(px - cx, pz - cz);
}

bool RectXZWithGatesConstraint::SegmentInsidePassesThroughAnyHole2D(const Vector3& aInside,
																	const Vector3& bInside) const noexcept {
	const auto& holes = holeSrc_->GetHoles();
	for (const auto& h : holes) {
		if (h.radius <= 0.0f) continue;
		float d = DistancePointToSegment2D(h.position.x, h.position.z,
										   aInside.x, aInside.z, bInside.x, bInside.z);
		if (d <= h.radius) return true; // 線分が穴円を横切る
	}
	return false;
}

Vector3 RectXZWithGatesConstraint::FilterMove(const Vector3& from, const Vector3& to) const {
	const bool fromInside = IsPointInsideRectXZ(from);
	const bool toInside = IsPointInsideRectXZ(to);

	// 完全に矩形に関与しない or 中→外 or 中→中 は検閲不要
	if ((!fromInside && !toInside)) {
		// 外→外だが交差しないならOK。交差するなら後段で処理
	} else if (fromInside && !toInside) {
		return to; // 中から出るのは常に許可
	} else if (fromInside && toInside) {
		return to; // 中で動くのも許可
	}

	// 外→外 のとき、矩形を横切るかチェック
	float tEnter = 0, tExit = 0;
	Vector3 pEnter, pExit;
	if (!SegmentIntersectRect2D(from, to, tEnter, tExit, pEnter, pExit)) {
		return to; // 触れていない
	}

	// 線分の「矩形内部区間」 [tEnter, tExit] を穴が貫通していればOK
	if (SegmentInsidePassesThroughAnyHole2D(pEnter, pExit)) {
		return to; // 穴経由で通過OK
	}

	// 穴が無ければ、矩形に入る直前で停止
	float back = (std::max)(0.0f, tEnter - 1e-4f);
	Vector3 halted = {
		from.x + (to.x - from.x) * back,
		from.y + (to.y - from.y) * back,
		from.z + (to.z - from.z) * back
	};

	// 誤差で食い込まないよう微小に押し戻す（進行方向と逆へ）
	Vector3 step = halted - from;
	float   len = step.Length();
	if (len > 1e-6f) {
		Vector3 dir = step * (1.0f / len);
		halted = halted - dir * eps_;
	}
	return halted;
}