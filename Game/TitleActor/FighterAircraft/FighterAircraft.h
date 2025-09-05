#pragma once

/// engine
#include "BaseObject/BaseObject.h"

/// //////////////////////////////////////////////////
/// 戦闘機のモデルを表示する
/// //////////////////////////////////////////////////
class FighterAircraft : public BaseObject {
public:
	/// ==================================
	/// public : methods
	/// ==================================

	FighterAircraft(const Vector3& _position);
	~FighterAircraft() override;

	void Init() override;
	void Update() override;

private:
	/// ==================================
	/// private : objects
	/// ==================================

	Vector3 position_;

};

