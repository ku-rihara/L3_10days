#pragma once

/// engine
#include "struct/TransformationMatrix.h"
#include "ShadowMap/ShadowMapData.h"
#include "RectXZ.h"
#include "Box.h"

/// game
#include "../../BaseObject/BaseObject.h"
#include "Pipeline/Buffer/StructuredBuffer.h"
#include "Pipeline/Buffer/ConstantBuffer.h"
#include "Pipeline/Buffer/IndexBuffer.h"
#include "Pipeline/Buffer/VertexBuffer.h"
#include "BoundaryShard.h"
#include "Details/IDamageSurface.h"

/// @brief 境界に空ける穴
struct Hole {
	Vector3 position;
	float radius;
	float startRadius; // 穴が空いたときの半径
	float lifeTime;    // 穴が空いてからの経過時間
};

struct BoundaryVertex {
	Vector4 pos;    // xyz座標
	Vector2 uv;     // uv座標
	Vector3 normal; // 法線ベクトル
};


/// //////////////////////////////////////////////////////
/// 味方陣地と敵陣地を区切る壁
/// //////////////////////////////////////////////////////
class Boundary :
	public BaseObject,
	public IDamageSurface{

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

	/// ----- 他のメンバーが使いそうな関数 ----- ///

	/// 罅の追加、罅が一定時間経過したら穴を追加する処理が走る
	void AddCrack(const Vector3& _pos, float _damage);

	/// 現在出現している穴のリスト
	const std::vector<Hole>& GetHoles() const;

	/// 現在出現している罅のリスト
	const std::vector<Breakable>& GetBreakables() const;
	std::vector<Breakable>& GetBreakablesRef();

	/// 境界に出来た罅の全てを持っているクラスの取得
	BoundaryShard* GetBoundaryShard();

	/// 設置できる穴の最大数を設定(罅の最大数でもある)
	size_t GetMaxHoleCount() const;

	// AABB(厚みを極薄にする
	AABB GetWorldAabb(float halfThicknessY = 0.05f)const;

	void OnBulletImpact(const Contact& c, float damage)override;

	bool IsInHoleXZ(const Vector3& p, float radius)const;



	/// ----- おそらく大野(Boundary内部で)しか使わないであろう関数 ----- ///

	RectXZ GetRectXZWorld()const;
	void GetDividePlane(Vector3& outOrigin, Vector3& outNormal) const;

	/// 穴の追加、罅を追加していったら穴が追加される
	void AddHole(const Vector3& pos, float radius);

	ConstantBuffer<ShadowTransformData>& GetShadowTransformBufferRef();
	ConstantBuffer<float>& GetTimeBufferRef();

private:
	/// ===================================================
	/// private : objects
	/// ===================================================

	size_t maxHoleCount_ = 1024;
	std::vector<Hole> holes_;

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

	RectXZ localRectXZ_{ -1500.0f, 1500.0f, -1500.0f, 1500.0f };
};

