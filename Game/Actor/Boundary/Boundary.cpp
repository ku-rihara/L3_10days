#include "Boundary.h"

/// engine
#include "Dx/DirectXCommon.h"
#include "Frame/Frame.h"
#include "input/Input.h"
#include "3d/ModelManager.h"

Boundary* Boundary::GetInstance() {
	static Boundary instance;
	return &instance;
}

Boundary::Boundary() {


	indexBuffer_.Create(6, DirectXCommon::GetInstance()->GetDxDevice());
	vertexBuffer_.Create(4, DirectXCommon::GetInstance()->GetDxDevice());
	vertexBuffer_.SetVertices({
		{ { -1500, 0, +1500, 1 }, { 0, 0 }, { 0, 1, 0 } },
		{ { +1500, 0, +1500, 1 }, { 1, 0 }, { 0, 1, 0 } },
		{ { -1500, 0, -1500, 1 }, { 0, 1 }, { 0, 1, 0 } },
		{ { +1500, 0, -1500, 1 }, { 1, 1 }, { 0, 1, 0 } }
		});

	vertexBuffer_.Map();

	indexBuffer_.SetIndices({ 0, 1, 2, 2, 1, 3 });
	indexBuffer_.Map();

	/// buffer init
	holeBuffer_.Create(static_cast<uint32_t>(maxHoleCount_), DirectXCommon::GetInstance()->GetDxDevice());

	for (int i = 0; i < maxHoleCount_; i++) {
		int x = i % 16;
		int y = i / 16;
		AddHole({ (float)x * 100.0f - 75.0f, 0.0f, (float)y * 100.0f - 75.0f }, 32.0f);
	}

	transformBuffer_.Create(DirectXCommon::GetInstance()->GetDxDevice());
	shadowTransformBuffer_.Create(DirectXCommon::GetInstance()->GetDxDevice());
	timeBuffer_.Create(DirectXCommon::GetInstance()->GetDxDevice());
}

void Boundary::Init() {
	baseTransform_.Init();

	/// test
	ModelManager::GetInstance()->LoadModel("BoundaryShard.obj");
}

void Boundary::Update() {


	/// debugように
	if (Input::GetInstance()->TrrigerKey(DIK_P)) {
		AddHole({}, 100.0f);
	}

	if (Input::GetInstance()->TrrigerKey(DIK_O)) {
		AddHole({ 50.0f, 0.0f, 0.0f }, 100.0f);
	}


	/// holeの更新
	for (auto itr = holes_.begin(); itr != holes_.end(); ) {
		itr->lifeTime -= Frame::DeltaTime();

		float lerpT = itr->lifeTime / holeMaxLifeTime_;
		itr->radius = itr->startRadius * std::clamp(lerpT, 0.0f, 1.0f);

		/// 一定時間経過した穴は消す
		if (itr->lifeTime <= 0.0f) {
			itr = holes_.erase(itr);
		} else {
			++itr;
		}
	}


	/// bufferに詰める
	for (size_t i = 0; i < maxHoleCount_; i++) {
		if (holes_.size() <= i) {
			Hole emptyHole = { { 0, 0, 0 }, 0.0f };
			holeBuffer_.SetMappedData(i, emptyHole);
			continue;
		}

		holeBuffer_.SetMappedData(i, holes_[i]);
	}

	float timeScale = 0.2f;
	timeBuffer_.SetMappedData(timeBuffer_.GetMappingData() + (Frame::DeltaTime() * timeScale));

}

void Boundary::AddHole(const Vector3& pos, float radius) {
	/// 超過してたら追加しない
	if (holes_.size() >= maxHoleCount_) {
		return;
	}

	Hole hole = {
		.position = pos,
		.radius = radius,
		.startRadius = radius,
		.lifeTime = holeMaxLifeTime_
	};

	holes_.emplace_back(hole);
}

void Boundary::AddCrack(const Vector3& _pos, float _radius, float _damage) {
	
	/// 他の亀裂と比較、近かったらその亀裂のlifeを減らす
	bool isNearCrack = false;
	for (size_t i = 0; i < cracks_.size(); i++) {
		Crack& otherCrack = cracks_[i];
		float distance = (otherCrack.position - _pos).Length();
		if (distance < otherCrack.radius) {
			otherCrack.life -= _damage;
			isNearCrack = true;
		}
	}


	/// 近くに亀裂がなかったら新しい亀裂を追加
	if(!isNearCrack) {
		Crack crack = {
			.position = _pos,
			.radius = _radius,
			.life = 1.0f
		};
		cracks_.emplace_back(crack);
	}
}

const std::vector<Hole>& Boundary::GetHoles() const {
	return holes_;
}

const std::vector<Crack>& Boundary::GetCracks() const {
	return cracks_;
}

std::vector<Crack>& Boundary::GetCracksRef() {
	return cracks_;
}
