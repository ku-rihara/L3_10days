#include "BoundaryBreaker.h"

#include "utility/ParameterEditor/GlobalParameter.h"
#include "Actor/Station/Base/BaseStation.h"
#include "Actor/NPC/Bullet/FireController/NpcFierController.h"
#include "Actor/Player/Bullet/BasePlayerBullet.h"
#include "Actor/ExpEmitter/ExpEmitter.h"

#include "Frame/Frame.h"
#include <cmath>
#include <algorithm>

BoundaryBreaker::BoundaryBreaker() = default;
BoundaryBreaker::~BoundaryBreaker() = default;

// ==============================
// 初期化
// ==============================
void BoundaryBreaker::Init(){
	BaseObject::Init();

	obj3d_.reset(Object3d::CreateModel("BoundaryBreaker.obj"));
	obj3d_->transform_.parent_ = &baseTransform_;
	baseTransform_.scale_ = Vector3(5.0f,5.0f,5.0f);

	fireController_ = std::make_unique<NpcFireController>();
	fireController_->Init();

	// GlobalParameter
	globalParam_ = GlobalParameter::GetInstance();
	if (globalParam_){
		globalParam_->CreateGroup(groupName_,true);
		globalParam_->ClearBindingsForGroup(groupName_);
		BindParms();
		globalParam_->SyncParamForGroup(groupName_);
	}

	// アンカー未設定時の補完
	if (anchorPosition_.x == 0.0f && anchorPosition_.y == 0.0f && anchorPosition_.z == 0.0f){
		anchorPosition_ = pRivalStation_
							  ? pRivalStation_->GetWorldPosition()
							  : GetWorldPosition();
	}

	AABBCollider::Init();
	SetCollisionScale(baseTransform_.scale_);
}

// ==============================
// 更新：フェーズ分け無し
// ==============================
void BoundaryBreaker::Update(){
	if (fireController_) fireController_->Tick();

	Move();
	Shoot();

	BaseObject::Update();
	cTransform_.translation_ = GetWorldPosition();
	cTransform_.UpdateMatrix();
}

void BoundaryBreaker::OnCollisionEnter(BaseCollider* other) {

	/// 
	if (BasePlayerBullet* bullet = dynamic_cast<BasePlayerBullet*>(other)) {
		hp_ -= bullet->GetParameter().damage;
		if(hp_ <= 0.0f) {
			hp_ = 0.0f;
			//Deactivate();
			/// エフェクトの生成
			ExpEmitter::GetInstance()->Emit(GetWorldPosition());
		}
	}
}

// ==============================
// パラメータバインド
// ==============================
void BoundaryBreaker::BindParms(){
	if (!globalParam_) return;

	// 速度・角速度
	globalParam_->Bind(groupName_,"speed",&speed_);
	globalParam_->Bind(groupName_,"maxAngularSpeedDeg",&maxAngularSpeedDeg_);
	globalParam_->Bind(groupName_,"useBaseRadiusForSpeed",&useBaseRadiusForSpeed_);

	// 射撃
	globalParam_->Bind(groupName_,"shootInterval",&shootInterval_);

	// ジオメトリ
	globalParam_->Bind(groupName_,"turningRadius",&turningRadius_);

	// ふわふわ・呼吸半径
	globalParam_->Bind(groupName_,"floatAmplitude",&floatAmplitude_);
	globalParam_->Bind(groupName_,"floatFreq",&floatFreq_);
	globalParam_->Bind(groupName_,"radiusWobbleAmp",&radiusWobbleAmp_);
	globalParam_->Bind(groupName_,"radiusWobbleFreq",&radiusWobbleFreq_);
}

// ==============================
// 移動（ふわふわ＋半径呼吸＋線速→角速変換＋角速上限）
// ==============================
void BoundaryBreaker::Move(){
	const float dt = Frame::DeltaTime();

	// 内部タイマー（TotalTime 不要）
	floatTime_ += dt;
	radiusTime_ += dt;

	// 半径の呼吸
	const float radiusOffset =
			std::sin(radiusTime_ * radiusWobbleFreq_ + phase_ * 1.37f) * radiusWobbleAmp_;
	const float rWobble = turningRadius_ + radiusOffset;

	// 線速度[m/s] → 角速度[rad/s]
	const float rForOmega = (std::max)(useBaseRadiusForSpeed_ ? turningRadius_ : rWobble,1.0f);
	float omega = speed_ / rForOmega; // [rad/s]

	// 角速度上限
	const float maxOmega = maxAngularSpeedDeg_ * (3.1415926535f / 180.0f);
	if (std::fabs(omega) > maxOmega){ omega = std::copysign(maxOmega,omega); }

	// 角度更新＆位置計算
	angle_ += omega * dt;
	const float a = angle_ + phase_;

	Vector3 newPos;
	newPos.x = anchorPosition_.x + std::cos(a) * rWobble;
	newPos.z = anchorPosition_.z + std::sin(a) * rWobble;
	newPos.y = anchorPosition_.y;

	// ふわふわ上下
	const float yOffset = std::sin(floatTime_ * floatFreq_ + phase_) * floatAmplitude_;
	newPos.y += yOffset;

	baseTransform_.translation_ = newPos;
}

// ==============================
// 射撃（クールダウンのみで常時発射）
// ==============================
void BoundaryBreaker::Shoot(){
	if (!fireController_) return;

	shootCooldown_ -= Frame::DeltaTime();
	if (shootCooldown_ > 0.0f) return;

	// ターゲット無しならテンポ維持だけ（空撃ちCD）
	if (!pRivalStation_){
		shootCooldown_ = shootInterval_;
		return;
	}

	const Vector3 pos = GetWorldPosition();
	const Vector3 dir = (pRivalStation_->GetWorldPosition() - pos).Normalize();
	fireController_->SpawnBreaker(pos,dir);

	shootCooldown_ = shootInterval_;
}