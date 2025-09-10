#pragma once

#include "RouteCollection.h"
#include "../../SplineFollower.h"
#include "Details/Faction.h"
#include "../NpcNavigator.h"

class RouteBinder {
public:
	RouteBinder() = default;
	~RouteBinder() = default;

	void SetRouteCollection(RouteCollection* rc) { routes_ = rc; }

	void Rebind(NpcNavigator& nav, FactionType faction, NpcNavigator::Role role) {
		if (!routes_) { nav.BindOrbitRoute(nullptr); return; }
		const RouteType rtype = ToRouteType(faction, role);
		Route* route = routes_->GetRoute(rtype);
		nav.BindOrbitRoute(route);
	}

private:
	static RouteType ToRouteType(FactionType faction, NpcNavigator::Role role) {
		switch (faction) {
			case FactionType::Ally:
				switch (role) {
					case NpcNavigator::Role::DefendBase: return RouteType::AllyDifence;
					case NpcNavigator::Role::Patrol:     return RouteType::AllyDifence;
					default:                              return RouteType::AllyDifence;
				}
			case FactionType::Enemy:
				switch (role) {
					case NpcNavigator::Role::DefendBase: return RouteType::EnemyDirence;
					case NpcNavigator::Role::Patrol:     return RouteType::EnemyDirence;
					default:                              return RouteType::EnemyDirence;
				}
			default:
				return RouteType::AllyDifence;
		}
	}

private:
	RouteCollection* routes_ = nullptr;
};
