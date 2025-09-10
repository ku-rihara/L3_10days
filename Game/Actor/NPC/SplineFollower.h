#pragma once

/// std
#include <random>

/// engine
#include "Vector3.h"
#include "Actor/NPC/Navigation/Route/Route.h"

class SplineFollower {
public:
	/* ========================================================================
	/*	public : types
	/* ===================================================================== */
	struct Output {
		Vector3 desiredDir{};   ///< このフレームの望ましい進行方向（正規化）
		float   plannedDist{};  ///< このフレームに進む予定距離（速度×dt×スケール）
		bool    switched{};     ///< 今フレームで候補切替したか
	};

public:
	/* ========================================================================
	/*	public : methods
	/* ===================================================================== */
	SplineFollower();
	~SplineFollower() = default;

	/// Route をバインドする（Route::Init済みのものを渡す）
	void BindRoute(Route* r);

	/// 最大旋回速度（deg/s）
	void SetMaxTurnRateDeg(float degPerSec);

	/// 初期候補をランダムに選択
	void SelectInitialRouteWeighted();

	/// 現在の候補が有効か
	bool HasUsableRoute() const noexcept;

	/// 現在位置に最も近いuへスナップ（初期化用途）
	void ResetAt(const Vector3& currentPos);

	/// 個体差と切替パラメータ
	void RandomizeIndividual(float speedJitterRatio,
	                         float lateralOffsetMax,
	                         float switchPeriodMean,
	                         float switchProb,
	                         uint32_t seed = 0);

	/// 1フレームの計画（望ましい向きと予定距離）
	Output Tick(const Vector3& currentPos,
	            const Vector3& currentHeading,
	            float baseSpeed,
	            float dt);

	/// 実際に移動した距離を報告して u を前進
	void Advance(float actualMovedDist);

private:
	/* ========================================================================
	/*	private : helpers
	/* ===================================================================== */
	Vector3 Sample_(float u) const;
	void    RebuildLengthEstimate_();
	static  float Wrap01_(float u);
	Vector3 TangentAt_(float u) const;
	bool    MaybeSwitch_(float dt, const Vector3& pos, bool atEnd);
	static  Vector3 RotateToward_(const Vector3& cur, const Vector3& des, float maxAngleRad);

private:
	/* ========================================================================
	/*	private : vars
	/* ===================================================================== */
	Route* route_ = nullptr;      ///< 追従対象（内部に複数候補を持つ）
	float  u_ = 0.0f;             ///< 現在の正規化位置 [0,1)

	// 速度→u 換算
	float curveLen_ = 1.0f;       ///< 近似全長
	float speedToDu_ = 20.0f;      ///< du = dist * speedToDu_

	// 個体差・切替
	float indivSpeedScale_ = 1.0f;
	float lateralOffset_   = 0.0f;
	float switchPeriod_    = 10.0f;// 5秒ごと
	float switchProb_      = 0.25f;
	float switchTimer_     = 0.0f;

	// 制御
	float maxTurnRateDeg_  = 180.0f;

	// ランダム
	std::mt19937 rng_{ std::random_device{}() };
};