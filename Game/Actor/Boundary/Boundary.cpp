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
	
	transformBuffer_.Create(DirectXCommon::GetInstance()->GetDxDevice());
	shadowTransformBuffer_.Create(DirectXCommon::GetInstance()->GetDxDevice());

	indexBuffer_.Create(6, DirectXCommon::GetInstance()->GetDxDevice());
	vertexBuffer_.Create(4, DirectXCommon::GetInstance()->GetDxDevice());
	vertexBuffer_.SetVertices(
		{
			{ { -100, 0, +100, 1 }, { 0, 0 }, { 0, 1, 0 } },
			{ { +100, 0, +100, 1 }, { 1, 0 }, { 0, 1, 0 } },
			{ { -100, 0, -100, 1 }, { 0, 1 }, { 0, 1, 0 } },
			{ { +100, 0, -100, 1 }, { 1, 1 }, { 0, 1, 0 } }
		}
	);

	vertexBuffer_.Map();

	indexBuffer_.SetIndices({ 0, 1, 2, 2, 1, 3 });
	indexBuffer_.Map();
}

void Boundary::Update() {

}
