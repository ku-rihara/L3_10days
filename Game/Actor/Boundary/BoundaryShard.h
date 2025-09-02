#pragma once

/// std
#include <vector>
#include <cstdint>

/// engine
#include "Vector4.h"
#include "Vector3.h"
#include "Vector2.h"
#include "struct/Transform.h"

/// game/pipeline
#include "Pipeline/Buffer/VertexBuffer.h"
#include "Pipeline/Buffer/IndexBuffer.h"


/// @brief 頂点データ
struct Vertex {
	Vector4 position;
	Vector2 uv;
	Vector3 normal;
};

/// @brief 境界に空ける穴の破片
struct Shard {
	VertexBuffer<Vertex> vertexBuffer;
	IndexBuffer indexBuffer;
	EulerTransform transform;
	bool active;
};

struct Breakable {
	float maxLife;
	float currentLife;
	int stage; /// 罅の段階
	std::vector<Shard> shards;
};

/// //////////////////////////////////////////////////////////
/// 境界の破片 Boundaryに定義する
/// //////////////////////////////////////////////////////////
class BoundaryShard {
public:
	/// ===================================================
	/// public : methods
	/// ===================================================

	BoundaryShard();
	~BoundaryShard();

	void Init();
	void Update();
	void Draw();

	void BindVBVAndIBV(ID3D12GraphicsCommandList* _cmdList, size_t _shardIndex);

private:
	/// ===================================================
	/// private : objects
	/// ===================================================

	std::vector<Shard> shards_;

};
