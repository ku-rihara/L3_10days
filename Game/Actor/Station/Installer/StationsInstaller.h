#pragma once
#include <memory>

class BaseStation;
class IUnitDirector;

namespace Installer {
	// 後方互換の2引数版（所有権は呼び出し側のまま）
	void InstallStations(BaseStation* ps, BaseStation* es);

	// UnitDirector を注入してから Rival/Init を行う版
	void InstallStations(BaseStation* ps, BaseStation* es, IUnitDirector* director);
}
