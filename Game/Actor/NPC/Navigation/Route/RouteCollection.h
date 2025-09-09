#pragma once

#include <unordered_map>
#include <memory>

class Route;
enum class RouteType;

class RouteCollection{
public:
	/* ========================================================================
	/*	public func
	/* ===================================================================== */
	RouteCollection();
	~RouteCollection();

	void Init();
	void Update();

	//--------- accessor -----------------------------------------------------
	Route* GetRoute(RouteType type) const;

private:
	/* ========================================================================
	/*	private func
	/* ===================================================================== */
	std::unordered_map<RouteType, std::unique_ptr<Route>> routes_;
};