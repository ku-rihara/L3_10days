#pragma once

#include "Vector3.h"
class BaseCollider;

struct Contact {
	Vector3 point;		// 着弾点
	Vector3 normal;		// 当たった面の法線
	float   t;			// 線分 [0,1] 上のヒット係数
	BaseCollider* collider = nullptr;
};