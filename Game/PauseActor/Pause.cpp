#include "Pause.h"

#include "input/Input.h"

void Pause::Init() {
	isPause_ = false;
}

void Pause::Update() {
	Input* input = Input::GetInstance();
	if (input->PushKey(DIK_P)) {
		isPause_ = !isPause_;
	}
}

void Pause::Draw() {

}