#include "BoundaryBreakerBullet.h"
#include "utility/ParameterEditor/GlobalParameter.h"
#include "Physics/SweepAabb.h"
#include "Actor/Boundary/Boundary.h"

// ===================================================
// 初期化
// ===================================================
void BoundaryBreakerBullet::Init() {
	if (isInitialized_) return;

	modelName_ = "cube.obj";
	groupName_ = "BoundaryBreakerBullet";

	NpcBullet::Init();	// 基底のセットアップ（モデル・親子付け等）
	BindParms();		// 速度などをバインド
	if (globalParam_) globalParam_->SyncParamForGroup(groupName_);
	speed_ = 50.f;
}

// ===================================================
// 更新
// ===================================================
void BoundaryBreakerBullet::Hit() {
	auto boundary = Boundary::GetInstance();

	if (boundary) {
		const AABB box = boundary->GetWorldAabb();
		auto hit = Sweep::SegmentSphereVsAabb(prevPos_, baseTransform_.translation_, radius_, box);
		if (hit) {
			// 穴内なら無効
			if (!boundary->IsInHoleXZ(hit->point, radius_)) {
				// 破壊通知（AddCrack 内部呼び出し）
				boundary->OnBulletImpact(*hit, damage_);
				Deactivate();
			}
		}
	}
}
