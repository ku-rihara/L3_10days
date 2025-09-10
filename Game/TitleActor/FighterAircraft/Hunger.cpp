#include "Hunger.h"

#include <numbers>

Hunger::Hunger() = default;
Hunger::~Hunger() = default;

void Hunger::Init() {
	BaseObject::Init();
	obj3d_.reset(Object3d::CreateModel("Humger.obj"));
	obj3d_->transform_.SetParent(&baseTransform_);
	baseTransform_.rotation_.y = std::numbers::pi_v<float>;
}

void Hunger::Update() {
	BaseObject::Update();
}
