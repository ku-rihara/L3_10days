#pragma once

#include <unordered_map>
#include <memory>

/// fwd
class Route;
enum class RouteType;

class RouteCollection{
public:
	/* ========================================================================
	/*	public func
	/* ===================================================================== */
	RouteCollection();
	~RouteCollection();

	void Init();    ///< 既定ディレクトリから全 RouteType を読み込む
	void Update();  ///< 各 Route の更新
	void DebugDraw(const class ViewProjection& vp)const;

	//--------- accessor -----------------------------------------------------
	Route* GetRoute(RouteType type) const; ///< 見つからなければ nullptr

private:
	/* ========================================================================
	/*	private vars
	/* ===================================================================== */
	std::unordered_map<RouteType, std::unique_ptr<Route>> routes_;
};