#pragma once
// engine
#include "utility/ParameterEditor/GlobalParameter.h"

// game
#include "BaseObject/BaseObject.h"
#include "Details/Faction.h"

class BaseStation;

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

	// ---- 調整項目 ----
	virtual void BindParms();
	virtual void LoadData();
	virtual void SaveData();

private:
	void Move();

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
	Vector3 Velocity_{};
};
