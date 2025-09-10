#include "NpcBullet.h"

#include "utility/ParameterEditor/GlobalParameter.h"
#include "Frame/Frame.h"
#include "Box.h"

NpcBullet::NpcBullet() {
	AABBCollider::Init();
	cTransform_.translation_ = { 0, -1000.0f, 0 }; // 見えないところに移動
	AABBCollider::isAdaptCollision = false;
	baseTransform_.Init();
	baseTransform_.translation_ = { 0, -1000.0f, 0 }; // 見えないところに移動
	baseTransform_.UpdateMatrix();
}

/// ===================================================
/// 初期化
/// ===================================================
void NpcBullet::Init() {
	globalParam_ = GlobalParameter::GetInstance();

	globalParam_->CreateGroup(groupName_, true);
	globalParam_->ClearBindingsForGroup(groupName_);

	globalParam_->SyncParamForGroup(groupName_);

	//transformの初期化
	obj3d_.reset(Object3d::CreateModel(modelName_));
	BaseObject::Init();
	obj3d_->transform_.parent_ = &baseTransform_;
	isInitialized_ = true;
	speed_ = 150.0f;

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
	Move();

	Hit();

	lifeRemain_ -= Frame::DeltaTime();
	if (lifeRemain_ <= 0.0f) {
		Deactivate(); 
	}

	BaseObject::Update();
}

/// ===================================================
/// 移動
/// ===================================================
void NpcBullet::Move() {
	baseTransform_.translation_ += dir_ * speed_ * Frame::DeltaTime();
}

void NpcBullet::Hit() {}

void NpcBullet::OnCollisionEnter(BaseCollider* other) {
	if (!isInitialized_) {
		return;
	}

	/// 何かに衝突したら消える
	if (dynamic_cast<NpcBullet*>(other)) return;

	Deactivate();
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