#pragma once
// engine
#include "utility/ParameterEditor/GlobalParameter.h"

// game
#include "BaseObject/BaseObject.h"
#include "Details/Faction.h"
#include "Navigation/NpcNavigator.h"
#include "Navigation/MoveConstraint.h"
#include "Navigation/RectXZWithGatesConstraint.h"

class BaseStation;
class Boundary;
class NpcFireController;
struct Hole;

class NPC : public BaseObject {
public:
	/// ===================================================
	///  public func
	/// ===================================================
	NPC() = default;
	~NPC() override = default;

	void Init() override;
	void Update() override;

	// --------- accessor --------------------------------
	void SetTarget(const BaseStation* target);
	void SetFaction(FactionType faction);
	bool GetIsAlive() const { return isActive_; }
	void SetFireControl(NpcFireController* fc) { pFireController = fc; }

	void Activate();
	void Deactivate();
	void TryFire();

	// 防衛アンカー（防衛時の旋回中心）を指定/解除
	void SetDefendAnchor(const Vector3& p);
	void ClearDefendAnchor();

	// ---- 調整項目 ----
	virtual void BindParms();
	virtual void LoadData();
	virtual void SaveData();

private:
	// 内部処理（末尾アンスコ無し）
	void Move();
	void StartOrbit(const Vector3& center);

protected:
	/// ===================================================
	///  protected-like variable
	/// ===================================================
	NpcFireController* pFireController = nullptr;
	// ---- param ----
	GlobalParameter* globalParam_ = nullptr;   //< 調整項目用
	std::string groupName_;                    //< 調整項目グループ名
	const std::string fileDirectory_ = "GameActor/NPC";

	// パラメータ
	float maxHP_ = 10.0f;      //< 最大hp
	float speed_ = 5.0f;       //< 基本移動速度（Navigator に渡す
	float shootInterval_ = 5.0f;
	float shootCooldown_ = 1.0f;
	// ---- game ----
	const BaseStation* target_ = nullptr;
	FactionType faction_{};

	bool isActive_ = true;
	bool isInitialized_ = false;

	// 防衛アンカー
	bool    hasDefendAnchor_ = false;
	Vector3 defendAnchor_{};

	// ---- 航法（純ロジック）----
	NpcNavConfig navConfig_{
		/*speed*/            18.0f,
		/*arriveSlowRadius*/ 3.0f,
		/*minHoleRadius*/    8.0f,
		/*passFrac*/         0.25f,
		/*retargetInterval*/ 0.2f,
		/*orbitRadius*/      20.0f,
		/*orbitAngularSpd*/  1.2f
	};
	NpcNavigator navigator_{ navConfig_ };

	// ---- 制約（境界板 XZ + 穴ゲート）----
	struct BoundaryHoleSource : IHoleSource {
		const Boundary* boundary = nullptr;
		const std::vector<Hole>& GetHoles() const override;
	} holeSource_;

	std::unique_ptr<IMoveConstraint> moveConstraint_; // RectXZWithGatesConstraint を格納
};
