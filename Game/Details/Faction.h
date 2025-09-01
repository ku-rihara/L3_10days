#pragma once

enum class FactionType {
	Ally,			//< 味方
	Enemy,			//< 敵
	Neutral,		//< 中立
};

inline bool IsEnemy(FactionType a, FactionType b) {
	return a != FactionType::Neutral && b != FactionType::Neutral && a != b;
}