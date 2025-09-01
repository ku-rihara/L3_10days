#include "Boundary.h"

Boundary::Boundary() {}

void Boundary::Init() {
	obj3d_.reset(Object3d::CreateModel("Boundary"));
	baseTransform_.Init();
}

void Boundary::Update() {

}
