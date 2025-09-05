#include "BoundaryBreakerInstaller.h"

#include "Actor/NPC/BoundaryBreaker/BoundaryBreaker.h"

void Installer::InstallBoundaryBreaker(std::vector<std::unique_ptr<BoundaryBreaker>>& boundaryBreakers,
									   const Vector3& stationPos, int spawnNum,
									   const BaseStation* rivalStation){
	boundaryBreakers.resize(spawnNum);

	float r = 100.0f;

	const float angleStep = 2.0f * 3.14159265f / static_cast<float>(spawnNum);

	for (int i = 0; i < spawnNum; ++i){
		const float angle = angleStep * i;

		auto boundaryBreaker = std::make_unique<BoundaryBreaker>();
		boundaryBreaker->Init();
		boundaryBreaker->SetAnchorPoint(stationPos);
		boundaryBreaker->SetRivalStation(rivalStation);

		boundaryBreaker->SetPhase(angle);
		boundaryBreaker->SetRadius(r);


		boundaryBreakers[i] = std::move(boundaryBreaker);
	}
}