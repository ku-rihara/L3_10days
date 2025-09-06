#pragma once
#include "NpcBullet.h"

class NpcHomingBullet:public NpcBullet{
public:
	/// ===================================================
	/// public func
	/// ===================================================
	NpcHomingBullet() = default;
	virtual ~NpcHomingBullet()override = default;

	void Init()override;
	void Update() override;

	//=== accessor ========================================
	void SetTarget(const BaseObject* target);

private:
	/// ===================================================
	/// private func
	/// ===================================================
	void BindParms()override;
	void SteerToTarget(float dt);


private:
	/// ===================================================
	/// private variable
	/// ===================================================
	const BaseObject* target_ = nullptr;

	//=== 調整項目 =========================================
	float turnRateDegPerSec_ = 180.0f;	//< 1秒あたり向きを変えられる最大角度
	float loseAngleDeg_ = 75.0f;		//< これ以上の偏差角でロックを外す
	float loseDistance_ = 800.0f;		//< これ以上離れたらロックを外す
	bool  keepLockInsideCone_ = true;	//< true:ロック解除でそのまま直線に進む,false:外れても追尾を続けて緩く旋回して角度が
};