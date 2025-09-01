#pragma once

/// game
#include "../../BaseObject/BaseObject.h"
#include "Pipeline/Buffer/StructuredBuffer.h"
#include "Pipeline/Buffer/ConstantBuffer.h"
#include "Pipeline/Buffer/IndexBuffer.h"
#include "Pipeline/Buffer/VertexBuffer.h"

/// buffer data
#include "struct/TransformationMatrix.h"
#include "ShadowMap/ShadowMapData.h"

/// @brief 境界に空ける穴
struct Hole {
	Vector3 position;
	float radius;
	float startRadius = 0.0f; // 穴が空いたときの半径
	float lifeTime = 0.0f; // 穴が空いてからの経過時間
};

struct BoundaryVertex {
	Vector4 pos;    // xyz座標
	Vector2 uv;     // uv座標
	Vector3 normal; // 法線ベクトル
};


/// //////////////////////////////////////////////////////
/// 味方陣地と敵陣地を区切る壁
/// //////////////////////////////////////////////////////
class Boundary : public BaseObject {
	friend class BoundaryPipeline;
	friend class BoundaryEdgePipeline;

	Boundary();
	~Boundary() = default;
public:
	/// ===================================================
	/// public : methods
	/// ===================================================

	/// シングルトンインスタンスのゲット
	static Boundary* GetInstance();

	void Init() override;
	void Update() override;

	void AddHole(const Vector3& pos, float radius);

	const std::vector<Hole>& GetHoles() const;

private:
	/// ===================================================
	/// private : objects
	/// ===================================================

	size_t maxHoleCount_ = 128;
	std::vector<Hole> holes_;

	float holeMaxLifeTime_ = 16.0f;

	/// vbv, ibv
	IndexBuffer indexBuffer_;
	VertexBuffer<BoundaryVertex> vertexBuffer_;


	/// vertex shader buffers
	ConstantBuffer<TransformationMatrix> transformBuffer_;
	ConstantBuffer<ShadowTransformData> shadowTransformBuffer_;

	/// pixel shader buffers
	StructuredBuffer<Hole> holeBuffer_;
	ConstantBuffer<float> timeBuffer_;

};

