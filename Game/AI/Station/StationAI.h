#pragma once
#include <algorithm>
#include <cmath>
#include "Vector3.h"
#include "Actor/Station/UnitDirector/IUnitDirector.h"

class BaseStation;

struct StationAiConfig {
	// 出力レンジ & しきい（攻め抑制・防衛厚め）
	float minAttack    = 0.06f; // 最低でも少数が攻める
	float maxAttack    = 0.55f; // 攻め上限を低めに
	int   minDefenders = 4;     // 常時4人は防衛

	// スムージング & リプラン（配分のブレを抑える）
	float smoothingSelf  = 4.0f; // 自HP EMA [1/sec]
	float smoothingRival = 4.0f; // 相手HP EMA
	float replanInterval = 0.8f; // 再配分をやや遅く
	float ratioDeadband  = 0.08f; // ±8%以内は据え置き

	// シグモイド温度（大=なだらか → 過剰に攻めへ振れにくい）
	float temperature = 1.2f;

	// 正規化（距離・被弾評価を防御寄りに）
	float refAttackRange = 150.0f; // この距離で D=1.0（距離ペナルティ強め）
	float dpsNormSelf    = 0.12f;  // 自被弾を“重い”と感じやすく
	float dpsNormRival   = 0.20f;  // 相手被弾では浮かれにくい

	// 重み（自陣・自隊を重視）
	struct W {
		float selfLowHp   = 1.4f;  //< S = 1 - HPs
		float rivalLowHp  = 0.7f;  //< R = 1 - HPr（相手低HPを過大評価しない）
		float selfDmgRate = 1.3f;  //< Sd（自被弾レート重視）
		float rivalDmgRate= 0.9f;  //< Rd（相手被弾は控えめ）
		float homeThreat  = 1.8f;  //< H（自陣危険度を強く反映）
		float distance    = 1.0f;  //< D（遠いほど攻めにくい）
		float biasAttack  = -0.10f;//< 攻撃バイアス（負で守備寄り）
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
	void ShowGui();
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

	struct DebugSnap {
		float S = 0.0f;   // 自HPの低さ = 1 - hpEmaSelf
		float R = 0.0f;   // 相手HPの低さ = 1 - hpEmaRival
		float H = 0.0f;   // homeThreat
		float D = 0.0f;   // 距離正規化（refAttackRange基準）
		float Sd = 0.0f;  // 自被弾レート（正規化）
		float Rd = 0.0f;  // 相手被弾レート（正規化）
		float U_attack = 0.0f;
		float U_defend = 0.0f;
		float pAtk     = 0.0f; // シグモイドの攻撃確率
		float dist     = 0.0f; // 実距離
	} dbg_;
};