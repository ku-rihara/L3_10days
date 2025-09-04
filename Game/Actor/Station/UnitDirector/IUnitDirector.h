#pragma once
#include "Vector3.h"

class BaseStation;

// Station からの「割当て要求」の受け口（実装はあなたの NpcPool / Spawner 側で）
class IUnitDirector {
public:
	virtual ~IUnitDirector() = default;

	// この拠点が現在「指揮可能」な概算ユニット数（空き＋切替可能を含めた見積り）
	virtual int GetControllableUnitCount(BaseStation* station) const = 0;

	// 防衛クオータ：station 近傍を守る人数
	virtual void AssignDefenseQuota(BaseStation* station, int count, const Vector3& defendAt) = 0;

	// 攻撃クオータ：rivalStation を攻撃する人数
	virtual void AssignAttackQuota(BaseStation* station, int count, BaseStation* rivalStation) = 0;
};
