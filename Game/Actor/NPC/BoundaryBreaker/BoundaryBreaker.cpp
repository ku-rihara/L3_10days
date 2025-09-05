#include "BoundaryBreaker.h"
#include "Frame/Frame.h"
#include "Actor/NPC/NPC.h"

//===================================================================*/
//				初期化
//===================================================================*/
void BoundaryBreaker::Init(){
	// モデル初期化
	obj3d_.reset(Object3d::CreateModel("Enemy.obj"));
	BaseObject::Init();

	obj3d_->transform_.parent_ = &baseTransform_;

	// グローバル変数
	globalParam_ = GlobalParameter::GetInstance();
	globalParam_->CreateGroup(groupName_, true);
	globalParam_->ClearBindingsForGroup(groupName_);
	BindParms();
	globalParam_->SyncParamForGroup(groupName_);

	// とりあえず色を変えてわかりやすく
	SetColor(Vector4(0.0f, 1.0f, 0.0f, 1.0f));
}

//===================================================================*/
//				更新
//===================================================================*/
void BoundaryBreaker::Update(){
	Move();
	BaseObject::Update();
}

//===================================================================*/
//				移動
//===================================================================*/
void BoundaryBreaker::Move(){
	const float dt = Frame::DeltaTime();

	angle_ += speed_ * dt;
	const float a = angle_ + phase_;

	Vector3 newPos;
	newPos.x = anchorPosition_.x + std::cos(a) * turningRadius_;
	newPos.z = anchorPosition_.z + std::sin(a) * turningRadius_;
	newPos.y = anchorPosition_.y;

	baseTransform_.translation_ = newPos;
}

void BoundaryBreaker::BindParms(){
	globalParam_->Bind(groupName_, "maxHP",         &maxHp_);
	globalParam_->Bind(groupName_, "speed",         &speed_);
	globalParam_->Bind(groupName_, "shootInterval", &shootInterval_);
}

//===================================================================*/
//				accessor
//===================================================================*/
void BoundaryBreaker::SetFactionType(const FactionType type){ faction_ = type; }
void BoundaryBreaker::SetAnchorPoint(const Vector3& point){ anchorPosition_ = point; }
void BoundaryBreaker::SetRivalStation(const BaseStation* station){ pRivalStation_ = station; }

void BoundaryBreaker::SetPhase(float rad){ phase_ = rad; }
void BoundaryBreaker::SetRadius(float r){ turningRadius_ = r; }

float BoundaryBreaker::GetTurningRadius() const { return turningRadius_; }