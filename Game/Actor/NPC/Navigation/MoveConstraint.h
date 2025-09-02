#pragma once
#include <vector>

class Vector3;
struct Hole;

// 通行制約の抽象
class IMoveConstraint {
public:
	virtual ~IMoveConstraint() = default;
	// from→to の移動を検閲し、必要に応じて修正した位置を返す
	virtual Vector3 FilterMove(const Vector3& from, const Vector3& to) const = 0;
};

// 穴一覧の供給元（Boundary を適合させる）
class IHoleSource {
public:
	virtual ~IHoleSource() = default;
	virtual const std::vector<Hole>& GetHoles() const = 0;
};