#include "PlayerEngineEffect.h"

// game/actor
#include "Actor/Player/Player.h"

/// engine
#include "Frame/Frame.h"
#include "random.h"

EngineEffect::EngineEffect(size_t _thisIndex) : thisIndex_(_thisIndex) {}
EngineEffect::~EngineEffect() = default;

void EngineEffect::Init() {
	BaseObject::Init();
	obj3d_.reset(Object3d::CreateModel("PlayerEngineEffect.obj"));
	obj3d_->transform_.SetParent(&baseTransform_);
	timer_ = 0.0f;
	randomOffset_ = Random::Range(-1.0f, 1.0f);

	obj3d_->objColor_.SetColor({ 1.0f, 0.0667f, 0.0f, 0.1f });

	/// thisIndexによってスケールを変える
	/// indexが大きいほどzに大きく、x,yに小さくする
	Vector3 scale = Vector3{ 0.15f, 0.15f, 5.0f } / 2.0f;
	scale.x += static_cast<float>(thisIndex_) * 0.1f / 2.0f;
	scale.y += static_cast<float>(thisIndex_) * 0.1f / 2.0f;
	scale.z -= static_cast<float>(thisIndex_) * 0.5f / 2.0f;

	baseTransform_.scale_ = scale;

	/// offset
	baseTransform_.translation_ = { 0, 0, -2.0f };
}

void EngineEffect::Update() {
	BaseObject::Update();

	timer_ += Frame::DeltaTime();

	/// uvスクロール
	uvRotateX_ = timer_ + randomOffset_;
	if (thisIndex_ % 2 == 0) {
		uvRotateX_ *= -1;
	}
	obj3d_->material_.materialData_->uvTransform = MakeTranslateMatrix({ uvRotateX_, 0, 0 });

}



PlayerEngineEffect::PlayerEngineEffect() = default;
PlayerEngineEffect::~PlayerEngineEffect() = default;

void PlayerEngineEffect::Init() {
	const int effectNum = 8;
	for (int i = 0; i < effectNum; ++i) {
		effects_.emplace_back(std::make_unique<EngineEffect>(i + 1));
		effects_.back()->Init();
	}
}

void PlayerEngineEffect::Update() {
	for (auto& effect : effects_) {
		effect->Update();
	}
}

void PlayerEngineEffect::SetPlayer(class Player* _ptr) {
	pPlayer_ = _ptr;
	for (size_t i = 0; i < effects_.size(); ++i) {
		effects_[i]->baseTransform_.SetParent(&pPlayer_->GetTransform());
	}
}