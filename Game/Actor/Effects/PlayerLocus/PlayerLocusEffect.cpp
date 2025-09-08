#include "PlayerLocusEffect.h"

/// game
#include "Actor/Player/Player.h"

PlayerLocusEffect::PlayerLocusEffect() = default;
PlayerLocusEffect::~PlayerLocusEffect() = default;

void PlayerLocusEffect::Init(Player* _ptr) {
	emitters_.resize(2);
	for (auto& emitter : emitters_) {
		emitter.reset(ParticleEmitter::CreateParticlePrimitive("Iocus", PrimitiveType::Box, 1000));
	}
	pPlayer_ = _ptr;
}

void PlayerLocusEffect::Update() {
	playerPos_ = pPlayer_->GetWorldPosition();

	/// Playerの回転を取得
	Quaternion qRot = pPlayer_->GetBaseQuaternion().Normalize();
	qRot = qRot * pPlayer_->GetObjQuaternion().Normalize();
	Vector3 eRot = qRot.ToEuler();

	Matrix4x4 matRot = MakeRotateMatrixQuaternion(qRot);
	Vector3 offsets[2] = {
		{ -3.3f, -1.0f, 3.0f },
		{ +3.3f, -1.0f, 3.0f }
	};

	/// 回転を反映
	for (size_t i = 0; i < emitters_.size(); i++) {
		emitters_[i]->SetEmitterRotate(eRot);
		emitters_[i]->SetTargetPosition(playerPos_ + TransformMatrix(offsets[i], matRot));
	}

	for (auto& emitter : emitters_) {
		emitter->SetTargetRotate(eRot);
		emitter->Update(); // 更新

		/// Playerの入力があったときのみ発生
		if (pPlayer_->GetAngleInput().Length() > 0.001f) {
			emitter->Emit(); // 発射
		}
	}
}

