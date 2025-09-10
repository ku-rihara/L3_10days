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
Vector3 RectXZWithGatesConstraint::FilterMove(const Vector3& from, const Vector3& to) const {
    blocked_ = false;

    // === プレーン定義 ===
    const float planeY = 0.0f;

    // from/to のプレーンに対する符号付き距離
    const float d0 = from.y - planeY;
    const float d1 = to.y - planeY;

    const float epsParallel = 1e-8f;
    if (std::fabs(d0 - d1) <= epsParallel || (d0 * d1) > 0.0f) {
        // ほぼ平行 or 同じ側 → 面を貫通していない
        return to;
    }

    const float denom = (to.y - from.y);
    if (std::fabs(denom) <= epsParallel) {
        // ほぼ平行（ただし上の条件で弾かれているはず）→ 安全側で通す
        return to;
    }
    float tCross = (planeY - from.y) / denom;
    tCross = std::clamp(tCross, 0.0f, 1.0f);

    // 交点
    const Vector3 crossP = {
        from.x + (to.x - from.x) * tCross,
        planeY, // by definition
        from.z + (to.z - from.z) * tCross
    };

    // 交点の (x,z) が矩形内かチェック（面の有効領域）
    const bool insideRect =
        (crossP.x >= rect_.minX && crossP.x <= rect_.maxX &&
         crossP.z >= rect_.minZ && crossP.z <= rect_.maxZ);

    if (!insideRect) {
        return to;
    }

    //  穴を通過していれば OK
    const auto& holes = holeSrc_->GetHoles();
    for (const auto& h : holes) {
        if (h.radius <= 0.0f) continue;
        const float dx = crossP.x - h.position.x;
        const float dz = crossP.z - h.position.z;
        const float dist = std::sqrt(dx * dx + dz * dz);
        if (dist <= h.radius) {
            // ゲートを通って面を横切った
            return to;
        }
    }

    //  穴を通らない → プレーンの直前で停止（面に衝突）
    blocked_ = true;

    // 進行方向
    const Vector3 move = to - from;
    const float   mlen = move.Length();
    Vector3 dir = (mlen > 1e-6f) ? (move * (1.0f / mlen)) : Vector3{ 0,0,0 };

    // ほんの少し手前（交点の tCross - 1e-4）で止める
    float tStop = (std::max)(0.0f, tCross - 1e-4f);
    Vector3 halted = {
        from.x + (to.x - from.x) * tStop,
        from.y + (to.y - from.y) * tStop,
        from.z + (to.z - from.z) * tStop
    };

    // 数値誤差で食い込まないよう、元側へ微小押し戻し
    if (mlen > 1e-6f) {
        halted = halted - dir * eps_;
    }
    return halted;
}

Vector3 RectXZWithGatesConstraint::FilterMoveReflect(const Vector3& from, const Vector3& to) const {
    blocked_ = false;

    // === プレーン定義（y=0） ===
    constexpr float planeY = 0.0f;
    const Vector3  n = {0,1,0}; // 法線

    // from/to のプレーンに対する符号付き距離
    const float d0 = from.y - planeY;
    const float d1 = to.y   - planeY;

    // ほぼ平行 or 同じ側 → 面を貫通していない
    const float epsParallel = 1e-8f;
    if (std::fabs(d0 - d1) <= epsParallel || (d0 * d1) > 0.0f) {
        return to;
    }

    // 面との交差パラメータ
    const float denom = (to.y - from.y);
    if (std::fabs(denom) <= epsParallel) {
        // 数値的に怪しい場合はそのまま通す
        return to;
    }
    float tCross = (planeY - from.y) / denom;
    tCross = std::clamp(tCross, 0.0f, 1.0f);

    // 交点
    const Vector3 crossP = {
        from.x + (to.x - from.x) * tCross,
        planeY,
        from.z + (to.z - from.z) * tCross
    };

    // 有効矩形内でのみ「境界」とみなす（外なら自由に通過してOK）
    const bool insideRect =
        (crossP.x >= rect_.minX && crossP.x <= rect_.maxX &&
         crossP.z >= rect_.minZ && crossP.z <= rect_.maxZ);
    if (!insideRect) {
        return to;
    }

    // 穴を通っていれば通過OK
    const auto& holes = holeSrc_->GetHoles();
    for (const auto& h : holes) {
        if (h.radius <= 0.0f) continue;
        const float dx = crossP.x - h.position.x;
        const float dz = crossP.z - h.position.z;
        const float dist = std::sqrt(dx*dx + dz*dz);
        if (dist <= h.radius) {
            return to; // ゲート通過
        }
    }

    // ==== プレーンに衝突：反射で押し戻す ====
    blocked_ = true;

    const Vector3 move = to - from;
    const float   totalLen = move.Length();

    // 交点まで進んだ残りの割合
    const float tRem = (std::max)(0.0f, 1.0f - tCross);
    Vector3     vRem = move * tRem; // 残りの“移動”ベクトル（速度の代用）

    // 接触点を元の側へ微小オフセット（食い込み防止）
    const float sideSign = (d0 >= 0.0f) ? +1.0f : -1.0f;
    const Vector3 contact = crossP + n * (sideSign * (eps_ + 1e-5f));

    // 残り移動の分解
    Vector3 vN  = n * (vRem.y);     // 法線成分（n=(0,1,0) なので vRem.y）
    Vector3 vT  = {vRem.x, 0.0f, vRem.z}; // 接線成分

    // 反射：法線成分を反転 * 反発係数、接線は摩擦で減衰
    const float restitution     = restitution_;      // 例: 0.2f（跳ね返り弱め）
    const float tangentFriction = tangentFriction_;  // 例: 0.9f（滑りつつ減衰）

    vN.y = -vN.y * restitution;
    vT.x *= tangentFriction;
    vT.z *= tangentFriction;

    // 反射後の残り移動
    Vector3 bounced = vT + Vector3{0.0f, vN.y, 0.0f};

    // さらにわずかに境界から離す“押し戻し”でスタック回避
    Vector3 pushAway = n * (sideSign * (0.5f * eps_ + 1e-4f));

    Vector3 out = contact + bounced + pushAway;

    // 念のため、反射後にまた面を跨ぎそうなら法線方向だけを残す
    // （数値誤差対策：同フレームでの二度目の交差を防ぐ）
    if ((out.y - planeY) * d0 <= 0.0f) {
        out.y = planeY + sideSign * (eps_ + 1e-4f);
    }

    // 総移動が極端に小さいと方向ロスでヒネリが出るので、最小量だけ離す
    if (totalLen < eps_) {
        out += n * (sideSign * (eps_ + 1e-4f));
    }

    return out;
}