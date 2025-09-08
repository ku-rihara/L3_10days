#include "StationAI.h"
#include "Actor/Station/Base/BaseStation.h"
#include <algorithm>
#include <cmath>

void StationAI::Initialize(BaseStation* owner, IUnitDirector* director) {
	owner_ = owner;
	director_ = director;
	hpEmaSelf_ = 1.0f;
	hpEmaRival_ = 1.0f;
	lastSelf_ = 1.0f;
	lastRival_ = 1.0f;
	replanT_ = 0.0f;
	out_ = {};
}

void StationAI::UpdateWeighted(float dt,
							   float selfHp, float selfMaxHp,
							   const Vector3& selfPos,
							   BaseStation* rival,
							   float homeThreat) {
	if (!owner_ || !director_) return;

	// Rival が居ない → 全員防衛
	if (!rival) {
		ApplyQuotas(selfPos, /*ratio*/0.0f);
		return;
	}

	// 値取得
	const float   rivalHp = GetHp(rival);
	const float   rivalMaxHp = GetMaxHp(rival);
	const Vector3 rivalPos = GetPos(rival);

	// === 正規化 ===
	const float sHP = (selfMaxHp > 0.0f) ? std::clamp(selfHp / selfMaxHp, 0.0f, 1.0f) : 0.0f;
	const float rHP = (rivalMaxHp > 0.0f) ? std::clamp(rivalHp / rivalMaxHp, 0.0f, 1.0f) : 0.0f;

	// === EMA ===
	auto ema = [&](float& emaV, float x, float k) {
		const float a = 1.0f - std::exp(-k * (std::max)(0.0f, dt));
		emaV += (x - emaV) * a;
	};
	ema(hpEmaSelf_, sHP, cfg_.smoothingSelf);
	ema(hpEmaRival_, rHP, cfg_.smoothingRival);

	// === 被弾レート（減少のみ）===
	const float dSelf = (std::max)(0.0f, (lastSelf_ - hpEmaSelf_) / (std::max)(1e-3f, dt));
	const float dRival = (std::max)(0.0f, (lastRival_ - hpEmaRival_) / (std::max)(1e-3f, dt));
	lastSelf_ = hpEmaSelf_;
	lastRival_ = hpEmaRival_;
	const float Sd = std::clamp(dSelf / (std::max)(1e-3f, cfg_.dpsNormSelf), 0.0f, 2.0f);
	const float Rd = std::clamp(dRival / (std::max)(1e-3f, cfg_.dpsNormRival), 0.0f, 2.0f);

	// === 特徴量 ===
	const float S = 1.0f - hpEmaSelf_;   // 自HPの低さ
	const float R = 1.0f - hpEmaRival_;  // Rival HPの低さ
	const float H = std::clamp(homeThreat, 0.0f, 1.0f);

	// 距離正規化（refAttackRange で D=1）
	const float dist = (rivalPos - selfPos).Length();
	const float D = std::clamp(dist / (std::max)(1e-3f, cfg_.refAttackRange), 0.0f, 2.0f);

	// === ユーティリティ ===
	const float U_attack =
		cfg_.w.rivalLowHp * R
		+ cfg_.w.rivalDmgRate * Rd
		- cfg_.w.selfLowHp * S
		- cfg_.w.selfDmgRate * Sd
		- cfg_.w.homeThreat * H
		- cfg_.w.distance * D
		+ cfg_.w.biasAttack;

	const float U_defend =
		cfg_.w.selfLowHp * S
		+ cfg_.w.selfDmgRate * Sd
		+ cfg_.w.homeThreat * H
		- cfg_.w.rivalLowHp * R
		- cfg_.w.rivalDmgRate * Rd;

	// === シグモイド ===
	const float T = (std::max)(1e-3f, cfg_.temperature);
	const float delta = (U_attack - U_defend) / T;
	const float pAtk = 1.0f / (1.0f + std::exp(-delta));

	const float newRatio =
		std::clamp(cfg_.minAttack + (cfg_.maxAttack - cfg_.minAttack) * pAtk, 0.0f, 1.0f);

	// リプラン & デッドバンド
	replanT_ -= dt;
	if (replanT_ <= 0.0f) {
		replanT_ = cfg_.replanInterval;
		if (std::fabs(newRatio - out_.attackRatio) >= cfg_.ratioDeadband) {
			ApplyQuotas(selfPos, newRatio);
		}
	}
}

void StationAI::ApplyQuotas(const Vector3& defendAt, float ratio){
	out_.attackRatio = ratio;

	const int total = ( std::max ) (0, director_->GetControllableUnitCount(owner_));

	// 通常の計算（minDefenders と攻撃比で防衛数を決める）
	int defenders = ( std::max ) (cfg_.minDefenders,
								  static_cast< int >(std::round(total * (1.0f - out_.attackRatio))));
	defenders = std::min(defenders, total);

	// ★ここが追加：攻撃対象が居る場合は最低1体を攻撃に回す
	if (rivalCached_ != nullptr && total > 0){
		defenders = std::min(defenders, total - 1);
	}

	const int attackers = total - defenders;

	out_.desiredDefenders = defenders;
	out_.desiredAttackers = attackers;

	director_->AssignDefenseQuota(owner_, defenders, defendAt);
	director_->AssignAttackQuota(owner_, attackers, rivalCached_);
}
// ===== BaseStation 依存ヘルパ =====
float   StationAI::GetHp(BaseStation* s)    const { return s->GetHp(); }
float   StationAI::GetMaxHp(BaseStation* s) const { return s->GetMaxHp(); }
Vector3 StationAI::GetPos(BaseStation* s)   const { return s->GetPosition(); }
