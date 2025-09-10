#include "Hunger.h"

Hunger::Hunger() = default;
Hunger::~Hunger() = default;

void Hunger::Init() {
	BaseObject::Init();
	obj3d_.reset(Object3d::CreateModel("Humger.obj"));
}

void Hunger::Update() {
	BaseObject::Update();
}
