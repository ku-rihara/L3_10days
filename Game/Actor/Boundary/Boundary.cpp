#include "Boundary.h"

/// engine
#include "Dx/DirectXCommon.h"
#include "Frame/Frame.h"
#include "input/Input.h"
#include "3d/ModelManager.h"
#include "Actor/Boundary/Boundary.h"

Boundary* Boundary::GetInstance() {
	static Boundary instance;
	return &instance;
}

Boundary::Boundary() {
	indexBuffer_.Create(6, DirectXCommon::GetInstance()->GetDxDevice());
	vertexBuffer_.Create(4, DirectXCommon::GetInstance()->GetDxDevice());

	float scale = 5.0f;
	vertexBuffer_.SetVertices({
		{ { -1500 * scale, 0, +1500 * scale, 1 }, { 0, 0 }, { 0, 1, 0 } },
		{ { +1500 * scale, 0, +1500 * scale, 1 }, { 1, 0 }, { 0, 1, 0 } },
		{ { -1500 * scale, 0, -1500 * scale, 1 }, { 0, 1 }, { 0, 1, 0 } },
		{ { +1500 * scale, 0, -1500 * scale, 1 }, { 1, 1 }, { 0, 1, 0 } }
		});

	localRectXZ_ = {
		-1500.0f * scale,
		+1500.0f * scale,
		-1500.0f * scale,
		+1500.0f * scale
	};

	vertexBuffer_.Map();

	indexBuffer_.SetIndices({ 0, 1, 2, 2, 1, 3 });
	indexBuffer_.Map();

	/// buffer init
	holeBuffer_.Create(static_cast<uint32_t>(maxHoleCount_), DirectXCommon::GetInstance()->GetDxDevice());

	transformBuffer_.Create(DirectXCommon::GetInstance()->GetDxDevice());
	shadowTransformBuffer_.Create(DirectXCommon::GetInstance()->GetDxDevice());
	timeBuffer_.Create(DirectXCommon::GetInstance()->GetDxDevice());

}

void Boundary::Init() {
	baseTransform_.Init();

	/// 
	boundaryShard_ = std::make_unique<BoundaryShard>();
	boundaryShard_->Init();

}

void Boundary::Update() {


	/// debugように
	if (Input::GetInstance()->TrrigerKey(DIK_P)) {
		AddCrack({}, 10.0f);
	}

	if (Input::GetInstance()->TrrigerKey(DIK_O)) {
		AddCrack({ 200.0f, 0.0f, 0.0f }, 10.0f);
	}


	boundaryShard_->Update();


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

ConstantBuffer<ShadowTransformData>& Boundary::GetShadowTransformBufferRef() {
	return shadowTransformBuffer_;
}

ConstantBuffer<float>& Boundary::GetTimeBufferRef() {
	return timeBuffer_;
}

bool Boundary::AddCrack(const Vector3& _pos, float _damage) {
	return boundaryShard_->AddBreakable(_pos, _damage);
}

const std::vector<Hole>& Boundary::GetHoles() const {
	return holes_;
}

RectXZ Boundary::GetRectXZWorld() const {
	RectXZ r = localRectXZ_;
	const float sx = baseTransform_.scale_.x;
	const float sz = baseTransform_.scale_.z;
	const float tx = baseTransform_.translation_.x;
	const float tz = baseTransform_.translation_.z;

	float x0 = r.minX * sx + tx;
	float x1 = r.maxX * sx + tx;
	float z0 = r.minZ * sz + tz;
	float z1 = r.maxZ * sz + tz;

	RectXZ out;
	out.minX = (std::min)(x0, x1);
	out.maxX = (std::max)(x0, x1);
	out.minZ = (std::min)(z0, z1);
	out.maxZ = (std::max)(z0, z1);
	return out;
}

void Boundary::GetDividePlane(Vector3& outOrigin, Vector3& outNormal) const {
	outOrigin = baseTransform_.translation_;
	outNormal = { 0.0f, 1.0f, 0.0f };
}

const std::vector<Breakable>& Boundary::GetBreakables() const {
	return boundaryShard_->GetBreakables();
}

std::vector<Breakable>& Boundary::GetBreakablesRef() {
	return boundaryShard_->GetBreakablesRef();
}

BoundaryShard* Boundary::GetBoundaryShard() {
	return boundaryShard_.get();
}

size_t Boundary::GetMaxHoleCount() const {
	return maxHoleCount_;
}

AABB Boundary::GetWorldAabb(float halfThicknessY) const {
	RectXZ r = GetRectXZWorld();
	Vector3 origin, n;
	GetDividePlane(origin, n);
	AABB box;
	box.min = { r.minX, origin.y - halfThicknessY, r.minZ };
	box.max = { r.maxX, origin.y + halfThicknessY, r.maxZ };
	return box;
}

bool Boundary::IsInHoleXZ(const Vector3& p, float radius) const {
	const auto& holes = GetHoles();
	for (const auto& h : holes) {
		Vector3 d{ p.x - h.position.x, 0.0f, p.z - h.position.z };
		float rr = h.radius + radius;
		if ((d.x * d.x + d.z * d.z) <= rr * rr) return true;
	}
	return false;
}

bool Boundary::OnBulletImpact(const Contact& c, float damage) {
	// 穴内なら無視（最終防衛）
	if (IsInHoleXZ(c.point, /*bullet r 不明なら0*/ 0.0f)) return false;
	return AddCrack(c.point, damage);
}