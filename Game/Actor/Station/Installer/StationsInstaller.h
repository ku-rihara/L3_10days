#pragma once

#include <memory>

class BaseStation;

namespace Installer {
	void InstallStations(const std::unique_ptr<BaseStation>& ps,
						 const std::unique_ptr<BaseStation>& es);
}