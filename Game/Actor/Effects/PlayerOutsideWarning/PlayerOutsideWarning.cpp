#include "PlayerOutsideWarning.h"

#include "Actor/Boundary/Boundary.h"
#include "PostEffect/PostEffectRenderer.h"

void PlayerOutsideWarning::Init() {
	Boundary* boundary = Boundary::GetInstance();
	RectXZ rect = boundary->GetRectXZWorld();
	warningRectSize_ = { rect.maxX - rect.minX, 1500.0f, rect.maxZ - rect.minZ };
}

void PlayerOutsideWarning::Update(Player* _player) {

	Vector3 playerPos = _player->GetPosition();
	if (std::abs(playerPos.x) > warningRectSize_.x ||
		std::abs(playerPos.y) > warningRectSize_.y ||
		std::abs(playerPos.z) > warningRectSize_.z) {
		// フィールド外
	} else {
		// フィールド内
	}
}

void PlayerOutsideWarning::Draw() {}
