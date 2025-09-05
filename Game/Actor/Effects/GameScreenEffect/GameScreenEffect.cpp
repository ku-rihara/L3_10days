#include "GameScreenEffect.h"

#include "Actor/Boundary/Boundary.h"
#include "PostEffect/PostEffectRenderer.h"

void GameScreenEffect::Init() {
	//Boundary* boundary = Boundary::GetInstance();
	//RectXZ rect = boundary->GetRectXZWorld();
	//warningRectSize_ = { rect.maxX - rect.minX, 1500.0f, rect.maxZ - rect.minZ };
	SetBaseColor({ 1.0f, 0.2f, 0.2f, 0.15f });
}

void GameScreenEffect::Update(Player* _player) {

	/// プレイヤーの位置が味方陣地か敵陣地かで色を変える
	float posY = _player->GetWorldPosition().y;
	if (posY > 0) {
		/// 味方陣地
		SetBaseColor({ 0.2f, 1.0f, 1.0f, 0.15f });
	} else {
		/// 敵陣地
		SetBaseColor({ 1.0f, 0.2f, 0.2f, 0.15f });
	}

	//Vector3 playerPos = _player->GetPosition();
	//if (std::abs(playerPos.x) > warningRectSize_.x ||
	//	std::abs(playerPos.y) > warningRectSize_.y ||
	//	std::abs(playerPos.z) > warningRectSize_.z) {
	//	// フィールド外
	//} else {
	//	// フィールド内
	//}
}

void GameScreenEffect::Draw() {}

const Vector4& GameScreenEffect::GetBaseColor() const {
	return baseColor_;
}

void GameScreenEffect::SetBaseColor(const Vector4& color) {
	baseColor_ = color;
}
