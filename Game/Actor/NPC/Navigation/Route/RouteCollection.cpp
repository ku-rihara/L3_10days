#include "RouteCollection.h"
#include "Route.h"
#include "3d/ViewProjection.h"

#include <filesystem>
#include <utility>

/////////////////////////////////////////////////////////////////////////////////////////
//		コンストラクタ/デストラクタ
/////////////////////////////////////////////////////////////////////////////////////////
RouteCollection::RouteCollection() = default;
RouteCollection::~RouteCollection() = default;

/////////////////////////////////////////////////////////////////////////////////////////
//		初期化
/////////////////////////////////////////////////////////////////////////////////////////
void RouteCollection::Init(){
	routes_.clear();

	// 基底ディレクトリ
	const std::string kBaseDir = "resources/GlobalParameter/GameActor/NpcRoute";

	const RouteType all[] = {
		RouteType::AllyDifence,
		RouteType::AllyAttack,
		RouteType::EnemyDirence,
		RouteType::EnemyAttack,
	};

	for (auto t : all) {
		auto r = std::make_unique<Route>();
		r->Init(t, kBaseDir);

		// ---- 陣営ごとに baseOffset を設定 ----
		Vector3 offset{0.0f, 0.0f, 0.0f};
		switch (t) {
		case RouteType::AllyDifence:
		case RouteType::AllyAttack:
			offset = {0.0f, -500.0f, 0.0f};
			break;
		case RouteType::EnemyDirence:
		case RouteType::EnemyAttack:
			offset = {0.0f,  500.0f, 0.0f};
			break;
		default:
			break;
		}
		r->SetBaseOffset(offset);
		routes_.emplace(t, std::move(r));
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
//		更新
/////////////////////////////////////////////////////////////////////////////////////////
void RouteCollection::Update(){
	for (auto& kv : routes_){
		if (kv.second){ kv.second->Update(); }
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
//		デバッグ描画
/////////////////////////////////////////////////////////////////////////////////////////
void RouteCollection::DebugDraw([[maybe_unused]]const ViewProjection& vp) const{
#ifdef _DEBUG
	for (auto& kv : routes_){
		if (kv.second){ kv.second->DrawDebug(vp); }
	}
#endif
}

/////////////////////////////////////////////////////////////////////////////////////////
//		accessor
/////////////////////////////////////////////////////////////////////////////////////////
Route* RouteCollection::GetRoute(RouteType type) const{
	auto it = routes_.find(type);
	if (it == routes_.end()) return nullptr;
	return it->second.get();
}