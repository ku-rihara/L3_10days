#include "Boundary.h"

Boundary* Boundary::GetInstance() {
	static Boundary instance;
	return &instance;
}

Boundary::Boundary() {}

void Boundary::Init() {
	baseTransform_.Init();
}

void Boundary::Update() {

}
