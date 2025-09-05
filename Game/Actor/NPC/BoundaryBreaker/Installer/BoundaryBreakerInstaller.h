#pragma once
#include <memory>
#include <vector>

class BaseStation;
class BoundaryBreaker;

namespace Installer{
void InstallBoundaryBreakers(std::vector<std::unique_ptr<BoundaryBreaker>>& boundaryBreakers,
							const BaseStation* stationPos,
							const BaseStation* rivalStation,
							int spawnNum);
}