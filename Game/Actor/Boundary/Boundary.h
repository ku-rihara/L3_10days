#pragma once

/// game
#include "../../BaseObject/BaseObject.h"

/// //////////////////////////////////////////////////////
/// 味方陣地と敵陣地を区切る壁
/// //////////////////////////////////////////////////////
class Boundary : public BaseObject {
public:
	/// ===================================================
	/// public : methods
	/// ===================================================

	static Boundary* GetInstance();

	Boundary();
	~Boundary() = default;

	void Init() override;
	void Update() override;

private:
	/// ===================================================
	/// private : objects
	/// ===================================================

};

