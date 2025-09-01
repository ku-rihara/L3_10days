#pragma once

// engine
#include "utility/ParameterEditor/GlobalParameter.h"

// game
#include "BaseObject/BaseObject.h"
#include "Details/Faction.h"

class NPC;

class BaseStation
	:public BaseObject{
public:
	/// ===================================================
	///  public func
	/// ===================================================
	BaseStation(const std::string& name = "UnnamedStation");
	virtual ~BaseStation()override;

	virtual void Init()override;
	virtual void Update()override;
	virtual  void ShowGui();

	// ---- 調整項目 ----
	virtual void BindParms();
	virtual void LoadData();
	virtual void SaveData();

	// ---- accessor ----
	void SetFaction(FactionType type);
	FactionType GetFactionType()const;

protected:
	virtual void SpawnNPC() = 0;
	void CleanupSpawnedList();

protected:
	/// ===================================================
	///  protected variable
	/// ===================================================
	//調整用
	GlobalParameter* globalParam_;		//< 調整項目用
	const std::string name_;			//< 調整項目グループ名
	const std::string fileDirectory_ = "GameActor/Station";
	
	// パラメータ
	Vector3 initialPosition_;			//< 初期座
	float maxLife_ = 100;				//< 最大hp
	float spawnInterbal_ = 5.0f;			//< スポーン間隔(5秒間隔
	int maxConcurrentUnits_ = 10;		//< 最大スポーン数(10体

protected:
	 // ---- game ----
	FactionType faction_;
	float hp_;
	float currentTime_ = 0;

	std::vector<std::unique_ptr<NPC>> spawned_;				//< スポーン済み
	std::weak_ptr<BaseStation> wRivalStation_;	//< ライバル拠点
};
