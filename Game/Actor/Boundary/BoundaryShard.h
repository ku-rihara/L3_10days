#pragma once

/// std
#include <vector>
#include <cstdint>
#include <string>

/// engine
#include "Matrix4x4.h"
#include "Vector4.h"
#include "Vector3.h"
#include "Vector2.h"
#include "struct/Transform.h"

/// game/pipeline
#include "Pipeline/Buffer/StructuredBuffer.h"
#include "Pipeline/Buffer/VertexBuffer.h"
#include "Pipeline/Buffer/IndexBuffer.h"


/// @brief 頂点データ
struct ShardVertex {
	Vector4 position;
	Vector2 uv;
	Vector3 normal;
};

/// @brief 境界に空ける穴の破片
struct Shard {
	VertexBuffer<ShardVertex> vertexBuffer;
	IndexBuffer indexBuffer;
	EulerTransform transform;
	bool active;
};

struct Breakable {
	float maxLife;
	float currentLife;
	int stage; /// 罅の段階
	std::vector<Shard> shards;
	StructuredBuffer<Matrix4x4> transformBuffer;
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

	void LoadShardModel(const std::string& _filepath);

	const std::vector<Breakable>& GetBreakables() const;

private:
	/// ===================================================
	/// private : objects
	/// ===================================================

	std::vector<Shard> loadedShards_; /// 読み込んだ破片モデル

	std::vector<Breakable> breakables_;

};
