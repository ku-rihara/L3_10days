#include "StationsInstaller.h"

#include "Actor/Station/Base/BaseStation.h"

void Installer::InstallStations(const std::unique_ptr<BaseStation>& ps,
								const std::unique_ptr<BaseStation>& es) {
	ps->Init();
	es->Init();

	ps->SetRivalStation(es.get());
	es->SetRivalStation(ps.get());
}
