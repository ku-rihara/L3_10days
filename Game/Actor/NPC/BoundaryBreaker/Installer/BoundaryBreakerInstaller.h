#pragma once
#include <memory>
#include <vector>

class BaseStation;
class Vector3;
class BoundaryBreaker;

namespace Installer{
void InstallBoundaryBreaker(std::vector<std::unique_ptr<BoundaryBreaker>>& boundaryBreakers,
							const Vector3& stationPos,
							int spawnNum,const BaseStation* rivalStation);
}