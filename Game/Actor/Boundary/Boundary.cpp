#include "Boundary.h"

/// engine
#include "Dx/DirectXCommon.h"
#include "Frame/Frame.h"
#include "input/Input.h"

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
}

void Boundary::Update() {


	/// debugように
	if(Input::GetInstance()->TrrigerKey(DIK_P)) {
		AddHole({}, 100.0f);
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

const std::vector<Hole>& Boundary::GetHoles() const {
	return holes_;
}
