#include "Boundary.h"

#include "Dx/DirectXCommon.h"

Boundary* Boundary::GetInstance() {
	static Boundary instance;
	return &instance;
}

Boundary::Boundary() {}

void Boundary::Init() {
	baseTransform_.Init();

	/// buffer init
	holeBuffer_.Create(128, DirectXCommon::GetInstance()->GetDxDevice());
	holeBuffer_.SetMappedData(0, { { 0, 0, 0 }, 5.0f });
	holeBuffer_.SetMappedData(1, { { 0, 0, 20 }, 10.0f });

	transformBuffer_.Create(DirectXCommon::GetInstance()->GetDxDevice());
	shadowTransformBuffer_.Create(DirectXCommon::GetInstance()->GetDxDevice());

	indexBuffer_.Create(6, DirectXCommon::GetInstance()->GetDxDevice());
	vertexBuffer_.Create(4, DirectXCommon::GetInstance()->GetDxDevice());
	vertexBuffer_.SetVertices({
		{ { -100, 0, +100, 1 }, { 0, 0 }, { 0, 1, 0 } },
		{ { +100, 0, +100, 1 }, { 1, 0 }, { 0, 1, 0 } },
		{ { -100, 0, -100, 1 }, { 0, 1 }, { 0, 1, 0 } },
		{ { +100, 0, -100, 1 }, { 1, 1 }, { 0, 1, 0 } }
		});

	vertexBuffer_.Map();

	indexBuffer_.SetIndices({ 0, 1, 2, 2, 1, 3 });
	indexBuffer_.Map();
}

void Boundary::Update() {
	for (size_t i = 0; i < holes_.size(); i++) {
		holeBuffer_.SetMappedData(i, holes_[i]);
	}
}

void Boundary::AddHole(const Vector3& pos, float radius) {
	Hole hole = { pos, radius };
	holes_.emplace_back(hole);
}
