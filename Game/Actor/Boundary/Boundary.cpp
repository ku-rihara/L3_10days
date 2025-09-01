#include "Boundary.h"

/// engine
#include "Dx/DirectXCommon.h"
#include "Frame/Frame.h"

Boundary* Boundary::GetInstance() {
	static Boundary instance;
	return &instance;
}

Boundary::Boundary() {}

void Boundary::Init() {
	baseTransform_.Init();


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
	holeBuffer_.Create(128, DirectXCommon::GetInstance()->GetDxDevice());

	for (int i = 0; i < 128; i++) {
		int x = i % 16;
		int y = i / 16;
		holeBuffer_.SetMappedData(i, { { (float)x * 100.0f - 75.0f, 0.0f, (float)y * 100.0f - 75.0f }, 32.0f });
	}

	transformBuffer_.Create(DirectXCommon::GetInstance()->GetDxDevice());
	shadowTransformBuffer_.Create(DirectXCommon::GetInstance()->GetDxDevice());
	timeBuffer_.Create(DirectXCommon::GetInstance()->GetDxDevice());
}

void Boundary::Update() {
	for (size_t i = 0; i < holes_.size(); i++) {
		holeBuffer_.SetMappedData(i, holes_[i]);
	}

	float timeScale = 0.2f;
	timeBuffer_.SetMappedData(timeBuffer_.GetMappingData() + (Frame::DeltaTime() * timeScale));

}

void Boundary::AddHole(const Vector3& pos, float radius) {
	Hole hole = { pos, radius };
	holes_.emplace_back(hole);
}
