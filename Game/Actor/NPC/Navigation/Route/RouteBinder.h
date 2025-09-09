#pragma once

#include "RouteCollection.h"
#include "../../SplineFollower.h"
#include "Details/Faction.h"
#include "../NpcNavigator.h"

class RouteBinder {
public:
	/* ========================================================================
	/*	public func
	/* ===================================================================== */
	RouteBinder() = default;
	~RouteBinder() = default;

	void SetRouteCollection(RouteCollection* rc){ routes_ = rc; }

	/// 役割や陣営が変わった時に呼ぶ
	void Rebind(SplineFollower& follower, FactionType faction, NpcNavigator::Role role){
		if (!routes_) { follower.BindRoute(nullptr); return; }
		const RouteType rtype = ToRouteType(faction, role);
		Route* route = routes_->GetRoute(rtype);
		follower.BindRoute(route);
	}

private:
	/* ========================================================================
	/*	private func
	/* ===================================================================== */
	static RouteType ToRouteType(FactionType faction, NpcNavigator::Role role){
		switch (faction){
		case FactionType::Ally:
			switch (role){
			case NpcNavigator::Role::AttackBase: return RouteType::AllyAttack;
			case NpcNavigator::Role::DefendBase: return RouteType::AllyDifence;
			case NpcNavigator::Role::Patrol:     return RouteType::AllyDifence; // 必要なら専用RouteTypeを追加
			default:                             return RouteType::AllyDifence;
			}
		case FactionType::Enemy:
			switch (role){
			case NpcNavigator::Role::AttackBase: return RouteType::EnemyAttack;
			case NpcNavigator::Role::DefendBase: return RouteType::EnemyDirence;
			case NpcNavigator::Role::Patrol:     return RouteType::EnemyDirence;
			default:                             return RouteType::EnemyDirence;
			}
		default:
			return RouteType::AllyDifence;
		}
	}

private:
	/* ========================================================================
	/*	private vars
	/* ===================================================================== */
	RouteCollection* routes_ = nullptr;
};