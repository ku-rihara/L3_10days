#pragma once
// engine
#include "utility/ParameterEditor/GlobalParameter.h"

// game
#include "BaseObject/BaseObject.h"
#include "Details/Faction.h"

class BaseStation;
struct Hole;

class NPC :
	public BaseObject {
public:
	/// ===================================================
	///  public func
	/// ===================================================
	NPC() = default;
	~NPC()override = default;
	void Init() override;
	void Update() override;

	//--------- accessor ----------------------------------
	void SetTarget(const BaseStation* target);
	void SetFaction(FactionType faction);
	bool GetIsAlive()const { return isActive_; }

	void Activate();
	void Deactivate();

	// ---- 調整項目 ----
	virtual void BindParms();
	virtual void LoadData();
	virtual void SaveData();

private:
	void Move();
	void StartOrbit(const Vector3& center);
	int  SelectBestHole(const std::vector<Hole>& holes,
						 const Vector3& npcPos,
						 const Vector3& tgtPos) const;
protected:
	/// ===================================================
	///  protected variable
	/// ===================================================
	
	// ---- param ----
	GlobalParameter* globalParam_;		//< 調整項目用
	std::string groupName_;				//< 調整項目グループ名
	const std::string fileDirectory_ = "GameActor/NPC";

	// パラメータ
	float maxHP_ = 10;					//< 最大hp
	float speed_ = 5.0f;				//< 移動速度

protected:
	 // ---- game ----
	const BaseStation* target_;
	FactionType faction_;

	bool isActive_;
	bool isInitialized_ = false;
	Vector3 Velocity_{};

	// ---- 穴 ----
	// ナビ状態
	enum class NavState { Orbit, ToHole, ToTarget };
	NavState state_ = NavState::Orbit;

	// 現在狙っている穴
	int      currentHoleIndex_ = -1;
	Vector3  currentHolePos_{};
	float    currentHoleRadius_ = 0.0f;

	// リターゲット抑制
	float retargetCooldown_ = 0.0f;
	float retargetInterval_ = 0.2f; // sec

	float arriveSlowRadius_ = 3.0f; // 減速開始距離

	// 穴の通過条件
	float minHoleRadius_ = 8.0f;     // これ未満は通行不可
	float passFrac_ = 0.25f;    // 半径の何割まで近づけば「通過扱い」

	// 旋回（穴が無いとき）
	float   orbitRadius_ = 20.0f;
	float   orbitAngularSpeed_ = 1.2f; // rad/sec
	float   orbitAngle_ = 0.0f;
	Vector3 orbitCenter_{};

};
