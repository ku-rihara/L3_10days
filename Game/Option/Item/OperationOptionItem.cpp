#include "OperationOptionItem.h"

#include "imgui.h"

/// engine
#include "base/TextureManager.h"

void OperationOptionItem::Init() {

	uint32_t texHandle = TextureManager::GetInstance()->LoadTexture(
		"./resources/Texture/Option/Controller.png"
	);

	controllerSprite_.reset(Sprite::Create(
		texHandle,
		{ 860.0f, 360.0f }, { 1, 1, 1, 1.0f })
	);

	controllerSprite_->anchorPoint_ = { 0.5f, 0.5f };
	controllerSprite_->SetScale(Vector2(1, 1) / 2.25.f);


	/// Frame
	texHandle = TextureManager::GetInstance()->LoadTexture(
		"./resources/Texture/Option/Check.png");
	frame_.reset(Sprite::Create(texHandle, { 640.0f, 360.0f }, { 1, 1, 1, 1.0f }));
	frame_->anchorPoint_ = { 0.5f, 0.5f };
	frame_->SetPosition({ 806.f, 568.f });
	frame_->SetScale(Vector2(1, 1) * 0.5f);
}

void OperationOptionItem::Update(size_t _currentIndex) {
	if (thisIndex_ != _currentIndex) {
		return;
	}

	///// imgui
	//static bool isFirst = true;
	//if (isFirst) {
	//	isFirst = false;
	//	return;
	//}
	//if (ImGui::Begin("operator option")) {
	//	static Vector2 pos = {640.0f, 360.0f};
	//	ImGui::DragFloat2("controller pos", &pos.x, 1.0f, 0.0f, 1280.0f);
	//	frame_->SetPosition(pos);
	//	ImGui::End();
	//}

}

void OperationOptionItem::Draw() {
	controllerSprite_->Draw();
	frame_->Draw();
}
