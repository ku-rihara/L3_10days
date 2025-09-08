#pragma once
#include <algorithm>
#include <cmath>
#include "Vector3.h"
#include "Actor/Station/UnitDirector/IUnitDirector.h"

class BaseStation;

struct StationAiConfig {
	// 出力レンジ & しきい
	float minAttack = 0.10f;   // 最小攻撃比
	float maxAttack = 0.80f;   // 最大攻撃比
	int   minDefenders = 2;    // 常に残す防衛人数

	// スムージング & リプラン
	float smoothingSelf = 3.0f;  // 自HP EMA [1/sec]
	float smoothingRival = 3.0f;  // 相手HP EMA
	float replanInterval = 0.5f;  // 再配分周期[s]
	float ratioDeadband = 0.05f; // 前回比と±5%以内は据え置き

	// シグモイド温度（大=なだらか、小=鋭い）
	float temperature = 0.8f;
	// 正規化
	float refAttackRange = 200.0f;  // この距離で D=1.0
	float dpsNormSelf = 0.15f;   // HP割合/秒 0.15 で “重い被弾”
	float dpsNormRival = 0.15f;

	// 重み
	struct W {
		float selfLowHp = 1.0f;		//< S = 1 - HPs
		float rivalLowHp = 1.2f;	//< R = 1 - HPr
		float selfDmgRate = 1.0f;	//< Sd（自被弾レート）
		float rivalDmgRate = 1.2f;	//< Rd（相手被弾レート）
		float homeThreat = 1.2f;	//< H  （自陣危険度 0..1）
		float distance = 0.7f;		//< D  （遠いほど攻めにくい）
		float biasAttack = 0.0f;	//< 攻撃バイアス
	} w;
};

struct StationAiOutput {
	float attackRatio = 0.5f;
	int   desiredDefenders = 0;
	int   desiredAttackers = 0;
};

class StationAI {
public:
	void Initialize(BaseStation* owner, IUnitDirector* director);

	void SetConfig(const StationAiConfig& c) { cfg_ = c; }
	StationAiConfig& Config() { return cfg_; }
	const StationAiOutput& GetLastOutput() const { return out_; }

	// RivalStation 前提の更新。Rival が無ければ全員防衛へフォールバック
	void UpdateWeighted(float dt,
						float selfHp, float selfMaxHp,
						const Vector3& selfPos,
						BaseStation* rival,
						float homeThreat /*0..1*/);

	// UpdateWeighted 内で参照する Rival をキャッシュできるよう外から設定可能に
	void SetRivalCached(BaseStation* r) { rivalCached_ = r; }

private:
	// BaseStation から値を取る（アクセサに合わせて実装）
	float   GetHp(BaseStation* s)    const;
	float   GetMaxHp(BaseStation* s) const;
	Vector3 GetPos(BaseStation* s)   const;

	void ApplyQuotas(const Vector3& defendAt, float ratio);

private:
	BaseStation* owner_ = nullptr;
	BaseStation* rivalCached_ = nullptr;
	IUnitDirector* director_ = nullptr;
	StationAiConfig cfg_{};
	StationAiOutput out_{};

	float hpEmaSelf_ = 1.0f;
	float hpEmaRival_ = 1.0f;
	float lastSelf_ = 1.0f;
	float lastRival_ = 1.0f;
	float replanT_ = 0.0f;
};