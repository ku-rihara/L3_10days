#pragma once

// engine
#include "utility/ParameterEditor/GlobalParameter.h"

// game
#include "BaseObject/BaseObject.h"

class BaseStation
	:public BaseObject{
public:
	/// ===================================================
	///  public func
	/// ===================================================
	BaseStation(const std::string& name = "UnnamedStation");
	virtual ~BaseStation()override = default;

	virtual void Init()override;
	virtual void Update()override;
	virtual  void ShowGui();

	//調整パラメータ
	virtual void BindParms();
	virtual void LoadData();
	virtual void SaveData();

protected:
	/// ===================================================
	///  protected variable
	/// ===================================================
	//調整用
	const std::string fileDirectory_ = "GameActor/Station";
	const std::string name_;			//< 調整項目グループ名
	GlobalParameter* globalParam_;		//< 調整項目用

	// パラメータ
	Vector3 initialPosition_;	//< 初期座
};
