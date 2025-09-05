#include "FighterAircraft.h"

/// std
#include <numbers>

FighterAircraft::FighterAircraft(const Vector3& _position) : position_(_position) {}
FighterAircraft::~FighterAircraft() {}

void FighterAircraft::Init() {
	obj3d_.reset(Object3d::CreateModel("Player.obj"));

	/// 180度回転させておく
	obj3d_->transform_.translation_ = position_;
	obj3d_->transform_.rotation_.y = std::numbers::pi_v<float>;
}

void FighterAircraft::Update() {
	obj3d_->Update();
	BaseObject::Update();
}
