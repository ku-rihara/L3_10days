#include "NpcBullet.h"

#include "utility/ParameterEditor/GlobalParameter.h"
#include "Frame/Frame.h"
#include "Box.h"
#include "Actor/Boundary/Boundary.h"
#include "Physics/SweepAabb.h"

/// ===================================================
/// 初期化
/// ===================================================
void NpcBullet::Init() {
	//初期化済みの場合は初期化しない
	if (isInitialized_) return;

	globalParam_ = GlobalParameter::GetInstance();

	globalParam_->CreateGroup(groupName_, true);
	globalParam_->ClearBindingsForGroup(groupName_);

	globalParam_->SyncParamForGroup(groupName_);

	//transformの初期化
	obj3d_.reset(Object3d::CreateModel("cube.obj"));
	BaseObject::Init();
	obj3d_->transform_.parent_ = &baseTransform_;
	isInitialized_ = true;
}

void NpcBullet::Init(const Vector3& dir) {
	Init();
	SetDirection(dir);
	Activate(); // 方向セット後に有効化＆寿命リセット
}

/// ===================================================
/// 更新
/// ===================================================
void NpcBullet::Update() {
	if (!isActive_) return;

	prevPos_ = baseTransform_.translation_;
	Move(); // 速度 * dt

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

	lifeRemain_ -= Frame::DeltaTime();
	if (lifeRemain_ <= 0.0f) Deactivate();

	BaseObject::Update();
}



void NpcBullet::Move() {
	baseTransform_.translation_ += dir_ * speed_ * Frame::DeltaTime();
}

/// ===================================================
/// パラメータ同期
/// ===================================================
void NpcBullet::BindParms() {
	globalParam_->Bind(groupName_, "speed", &speed_);
}

/// ===================================================
/// パラメータのロード、保存
/// ===================================================
void NpcBullet::LoadData() { globalParam_->LoadFile(groupName_, fileDirectory_); }
void NpcBullet::SaveData() { globalParam_->SaveFile(groupName_, fileDirectory_); }

/// ===================================================
/// 方向をセット
/// ===================================================
void NpcBullet::SetDirection(const Vector3& dir) { dir_ = dir.Normalize(); }
