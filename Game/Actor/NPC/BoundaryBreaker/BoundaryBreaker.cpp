#include "BoundaryBreaker.h"
#include "Frame/Frame.h"
#include "Actor/NPC/NPC.h"
#include "random.h"

BoundaryBreaker::~BoundaryBreaker() = default;
//===================================================================*/
//				初期化
//===================================================================*/
void BoundaryBreaker::Init(){
	// モデル初期化
	obj3d_.reset(Object3d::CreateModel("Enemy.obj"));
	BaseObject::Init();

	obj3d_->transform_.parent_ = &baseTransform_;

	fireController_ = std::make_unique<NpcFireController>();
	fireController_->Init();

	// グローバル変数
	globalParam_ = GlobalParameter::GetInstance();
	globalParam_->CreateGroup(groupName_, true);
	globalParam_->ClearBindingsForGroup(groupName_);
	BindParms();
	globalParam_->SyncParamForGroup(groupName_);

	// 弾のクールダウンをランダムで初期化
	shootCooldown_ = Random::Range(1.0f,shootInterval_);


	// とりあえず色を変えてわかりやすく
	SetColor(Vector4(0.0f, 1.0f, 0.0f, 1.0f));
}

//===================================================================*/
//				更新
//===================================================================*/
void BoundaryBreaker::Update(){
	fireController_->Tick();
	Move();
	Shoot();
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

//===================================================================*/
//				パラメータの動機
//===================================================================*/
void BoundaryBreaker::BindParms(){
	globalParam_->Bind(groupName_, "maxHP",         &maxHp_);
	globalParam_->Bind(groupName_, "speed",         &speed_);
	globalParam_->Bind(groupName_, "shootInterval", &shootInterval_);
}

//===================================================================*/
//				発射処理
//===================================================================*/
void BoundaryBreaker::Shoot(){
	if (!fireController_)return;
	shootCooldown_ -= Frame::DeltaTime();
	if (shootCooldown_ > 0.0f) return;
	if (!pRivalStation_) return;

	//ライバルのステーションに向けて発射する
	Vector3 dir = Vector3(pRivalStation_->GetWorldPosition() - GetWorldPosition()).Normalize();
	fireController_->SpawnBreaker(GetWorldPosition(),dir);
	shootCooldown_ = shootInterval_;
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