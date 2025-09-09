#include "SplineFollower.h"

/// std
#include <algorithm>
#include <limits>
#include <cmath>

#include "Navigation/Route/Route.h"

static inline float Clamp1(float v, float lo, float hi){ return (v < lo) ? lo : ((v > hi) ? hi : v); }
static inline float Rad(float deg){ return deg * 3.1415926535f / 180.0f; }

/////////////////////////////////////////////////////////////////////////////////////////
//      バインド
/////////////////////////////////////////////////////////////////////////////////////////
void SplineFollower::BindRoute(Route* r){
	route_ = r;
	u_ = 0.0f;
	SelectInitialRouteWeighted();
	RebuildLengthEstimate_();
	switchTimer_ = 0.0f;
}

/////////////////////////////////////////////////////////////////////////////////////////
//      最大旋回速度
/////////////////////////////////////////////////////////////////////////////////////////
void SplineFollower::SetMaxTurnRateDeg(float degPerSec){
	maxTurnRateDeg_ = (degPerSec > 0.f ? degPerSec : 180.f);
}

/////////////////////////////////////////////////////////////////////////////////////////
//      初期候補選択
/////////////////////////////////////////////////////////////////////////////////////////
void SplineFollower::SelectInitialRouteWeighted(){
	if (!route_ || route_->GetVariantCount() <= 0) return;
	route_->ChooseRandomVariant();
	RebuildLengthEstimate_();
}

/////////////////////////////////////////////////////////////////////////////////////////
//      有効チェック
/////////////////////////////////////////////////////////////////////////////////////////
bool SplineFollower::HasUsableRoute() const noexcept{
	return (route_ && route_->GetVariantCount() > 0 && route_->GetActiveIndex() >= 0);
}

/////////////////////////////////////////////////////////////////////////////////////////
//      近傍uへスナップ
/////////////////////////////////////////////////////////////////////////////////////////
void SplineFollower::ResetAt(const Vector3& currentPos){
	if (!HasUsableRoute()) return;

	// 粗サーチ→微調整
	const int   coarseN = 64;
	float bestU = 0.0f;
	float bestD2 = (std::numeric_limits<float>::max)();

	for (int i = 0; i < coarseN; ++i){
		float u = (i + 0.5f) / coarseN;
		Vector3 p = Sample_(u);
		Vector3 d = p - currentPos;
		float d2 = Vector3::Dot(d, d);
		if (d2 < bestD2){ bestD2 = d2; bestU = u; }
	}

	// 微調整（左右に小刻み探索）
	const float du = 1.0f / (coarseN * 8.0f);
	for (int k = -8; k <= 8; ++k){
		float u = Wrap01_(bestU + du * k);
		Vector3 p = Sample_(u);
		Vector3 d = p - currentPos;
		float d2 = Vector3::Dot(d, d);
		if (d2 < bestD2){ bestD2 = d2; bestU = u; }
	}
	u_ = Wrap01_(bestU);
}

/////////////////////////////////////////////////////////////////////////////////////////
//      個体差＆切替パラメータ
/////////////////////////////////////////////////////////////////////////////////////////
void SplineFollower::RandomizeIndividual(float speedJitterRatio,
                                         float lateralOffsetMax,
                                         float switchPeriodMean,
                                         float switchProb,
                                         uint32_t seed){
	if (seed != 0) rng_.seed(seed);

	std::uniform_real_distribution<float> U01(0.f,1.f);
	std::uniform_real_distribution<float> U(-1.f,1.f);

	indivSpeedScale_ = 1.f + Clamp1(speedJitterRatio, 0.f, 1.f) * U(rng_);
	lateralOffset_   = lateralOffsetMax * U(rng_);
	switchPeriod_    =(std::max)(0.5f, switchPeriodMean * (0.75f + 0.5f * U01(rng_)));
	switchProb_      = Clamp1(switchProb, 0.f, 1.f);
}

/////////////////////////////////////////////////////////////////////////////////////////
//      Tick（スプライン追従計画）
/////////////////////////////////////////////////////////////////////////////////////////
SplineFollower::Output
SplineFollower::Tick(const Vector3& currentPos,
                     const Vector3& currentHeading,
                     float baseSpeed,
                     float dt){
	Output o{};
	if (!HasUsableRoute()){
		o.desiredDir = currentHeading;
		o.plannedDist =(std::max)(0.0f, baseSpeed * dt);
		o.switched = false;
		return o;
	}

	// 予定距離と look-ahead
	const float planned =(std::max)(0.0f, baseSpeed * indivSpeedScale_ * dt);
	const float lookAheadDist = planned;

	// 先の点をサンプル（距離→du変換）
	const float duAhead = lookAheadDist * speedToDu_;
	const float uAhead  = Wrap01_(u_ + duAhead);

	Vector3 p  = Sample_(u_);
	Vector3 pL = Sample_(uAhead);

	// 横オフセットを適用（Up×TangentでRightを作る）
	Vector3 tan = TangentAt_(u_);
	Vector3 right = Vector3::Cross(Vector3::ToUp(), tan);
	if (right.Length() > 1e-6f){
		right = Vector3::Normalize(right);
		p  = p  + right * lateralOffset_;
		pL = pL + right * lateralOffset_;
	}

	Vector3 desired = Vector3::NormalizeOr(pL - currentPos, currentHeading);

	// 回頭制限
	const float maxTurn = Rad(maxTurnRateDeg_) * dt;
	Vector3 turned = RotateToward_(currentHeading, desired, maxTurn);

	o.desiredDir = turned;
	o.plannedDist = planned;
	o.switched = MaybeSwitch_(dt, currentPos, /*atEnd*/ false);

	return o;
}

/////////////////////////////////////////////////////////////////////////////////////////
//      Advance
/////////////////////////////////////////////////////////////////////////////////////////
void SplineFollower::Advance(float actualMovedDist){
	if (!HasUsableRoute() || actualMovedDist <= 0.0f) return;
	const float du = actualMovedDist * speedToDu_;
	u_ = Wrap01_(u_ + du);
}

/////////////////////////////////////////////////////////////////////////////////////////
//      サンプル（u）
/////////////////////////////////////////////////////////////////////////////////////////
Vector3 SplineFollower::Sample_(float u) const{
	if (!HasUsableRoute()) return {};
	return route_->Sample(Clamp1(u, 0.0f, 1.0f));
}

/////////////////////////////////////////////////////////////////////////////////////////
//      長さ見積もりの再構築
/////////////////////////////////////////////////////////////////////////////////////////
void SplineFollower::RebuildLengthEstimate_(){
	if (!HasUsableRoute()){
		curveLen_ = 1.0f;
		speedToDu_ = 1.0f;
		return;
	}
	const int N = 128;
	Vector3 prev = Sample_(0.0f);
	float acc = 0.0f;
	for (int i = 1; i <= N; ++i){
		float u = (float)i / N;
		Vector3 p = Sample_(u);
		acc += (p - prev).Length();
		prev = p;
	}
	curveLen_ = (acc > 1e-5f ? acc : 1.0f);
	speedToDu_ = 1.0f / curveLen_;
}

/////////////////////////////////////////////////////////////////////////////////////////
//      Wrap01
/////////////////////////////////////////////////////////////////////////////////////////
float SplineFollower::Wrap01_(float u){
	u = std::fmod(u, 1.0f);
	if (u < 0.0f) u += 1.0f;
	return u;
}

/////////////////////////////////////////////////////////////////////////////////////////
//      接線（微小差分）
/////////////////////////////////////////////////////////////////////////////////////////
Vector3 SplineFollower::TangentAt_(float u) const{
	const float eps = 1.0f / 1024.0f;
	Vector3 a = Sample_(Wrap01_(u));
	Vector3 b = Sample_(Wrap01_(u + eps));
	return Vector3::NormalizeOr(b - a, Vector3::ToForward());
}

/////////////////////////////////////////////////////////////////////////////////////////
//      候補切替
/////////////////////////////////////////////////////////////////////////////////////////
bool SplineFollower::MaybeSwitch_(float dt, const Vector3& /*pos*/, bool atEnd) {
	if (!HasUsableRoute() || route_->GetVariantCount() <= 1) return false;

	// スイッチのタイマー更新
	switchTimer_ += dt;
	const bool timeTrigger = (switchTimer_ >= switchPeriod_);
	if (!timeTrigger) return false;

	// 次周期へ
	switchTimer_ = 0.0f;

	// 確率判定
	std::uniform_real_distribution<float> U01(0.f, 1.f);
	if (U01(rng_) > switchProb_) return false;

	// ---- 切替前の情報を保持 ----
	const float oldLen = curveLen_;   // 旧バリアントの推定全長
	const float uBefore = u_;         // 旧u（正規化）

	// 端まで来ているなら、わずかに手前に戻してから切替（端スナップを避ける）
	float uForSwitch = uBefore;
	if (atEnd) {
		// 端から少しだけ戻す
		uForSwitch = (std::min)(uBefore, 0.995f);
	}

	// ---- バリアントを切替（uは維持前提）----
	route_->SwitchVariantKeepU(uForSwitch);

	// ---- 新バリアントの長さに合わせて u を再スケール（弧長だいたい維持）----
	RebuildLengthEstimate_();

	if (curveLen_ > 1e-6f && oldLen > 1e-6f) {
		// 旧：弧長 s = uBefore * oldLen
		const float s = uBefore * oldLen;
		// 新：uAfter ~ s / newLen
		const float uAfter = s / curveLen_;
		u_ = Wrap01_(uAfter);
	} else {
		// 長さが不明/極端に短い場合は単純にu維持
		u_ = Wrap01_(uBefore);
	}

	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////
//      回頭補助
/////////////////////////////////////////////////////////////////////////////////////////
Vector3 SplineFollower::RotateToward_(const Vector3& cur, const Vector3& des, float maxAngleRad){
	Vector3 a = Vector3::NormalizeOr(cur, Vector3::ToForward());
	Vector3 b = Vector3::NormalizeOr(des, Vector3::ToForward());
	float dot = std::clamp(Vector3::Dot(a, b), -1.0f, 1.0f);
	float theta = std::acos(dot);
	if (theta <= maxAngleRad) return b;
	float t = maxAngleRad / (theta + 1e-8f);
	Vector3 lerp = a * (1.0f - t) + b * t;
	return Vector3::NormalizeOr(lerp, b);
}