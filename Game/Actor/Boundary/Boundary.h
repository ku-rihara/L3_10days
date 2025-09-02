#pragma once

/// engine
#include "struct/TransformationMatrix.h"
#include "ShadowMap/ShadowMapData.h"

/// game
#include "../../BaseObject/BaseObject.h"
#include "Pipeline/Buffer/StructuredBuffer.h"
#include "Pipeline/Buffer/ConstantBuffer.h"
#include "Pipeline/Buffer/IndexBuffer.h"
#include "Pipeline/Buffer/VertexBuffer.h"
#include "BoundaryShard.h"


/// @brief 境界に空ける穴
struct Hole {
	Vector3 position;
	float radius;
	float startRadius; // 穴が空いたときの半径
	float lifeTime;    // 穴が空いてからの経過時間
};

/// @brief 境界が割れる前の亀裂
struct Crack {
	Vector3 position;
	float radius;
	float life; /// 0になったらHoleが生まれる
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
	void AddCrack(const Vector3& _pos, float _radius, float _damage);

	const std::vector<Hole>& GetHoles() const;
	const std::vector<Crack>& GetCracks() const;
	std::vector<Crack>& GetCracksRef();

	BoundaryShard* GetBoundaryShard();

private:
	/// ===================================================
	/// private : objects
	/// ===================================================

	size_t maxHoleCount_ = 128;
	std::vector<Hole> holes_;
	std::vector<Crack> cracks_;

	float holeMaxLifeTime_ = 16.0f;

	std::unique_ptr<BoundaryShard> boundaryShard_;


	/// ----- buffer ----- ///

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

