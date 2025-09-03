#include "StationsInstaller.h"
#include "Actor/Station/Base/BaseStation.h"
#include "Actor/Station/UnitDirector/IUnitDirector.h"

void Installer::InstallStations(BaseStation* ps, BaseStation* es) {
	if (!ps || !es) return;

	// 先に Rival を
	ps->SetRivalStation(es);
	es->SetRivalStation(ps);

	// 初期化
	ps->Init();
	es->Init();
}

void Installer::InstallStations(BaseStation* ps, BaseStation* es, IUnitDirector* director) {
	if (!ps || !es) return;

	ps->SetUnitDirector(director);
	es->SetUnitDirector(director);

	InstallStations(ps, es);
}
