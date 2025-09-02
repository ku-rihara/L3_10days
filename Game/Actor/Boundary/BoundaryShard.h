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
#include "struct/TransformationMatrix.h"

/// game/pipeline
#include "Pipeline/Buffer/StructuredBuffer.h"
#include "Pipeline/Buffer/ConstantBuffer.h"
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
	float phase = 0.0f;
	Vector3 offsetPos;
	Vector3 initPos;
	Vector3 offsetRotate;
	Vector3 initRotate;
	Vector3 normal;
	float randomSmallRotation;
};

struct Breakable {
	Vector3 position;
	float maxLife;
	float currentLife; /// 0になったら破片に分かれる and Holeが生まれる
	int stage; /// 罅の段階
	float radius; /// 罅の大きさ
	float frameTime = 0.0f; /// 罅ができてからの経過時間
	std::vector<Shard> shards;
};

/// //////////////////////////////////////////////////////////
/// 境界の破片 Boundaryに定義する
/// //////////////////////////////////////////////////////////
class BoundaryShard {
	friend class BoundaryShardPipeline;
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
	std::vector<Breakable>& GetBreakablesRef();

	const std::vector<Shard>& GetLoadedShards() const;

	void AddBreakable(const Vector3& _position, float _damage);

private:
	/// ===================================================
	/// private : objects
	/// ===================================================

	std::vector<Shard> loadedShards_; /// 読み込んだ破片モデル

	std::vector<Breakable> breakables_;
	StructuredBuffer<TransformationMatrix> breakableTransformBuffer_;
	ConstantBuffer<int> instanceCountBuffer_;

};
