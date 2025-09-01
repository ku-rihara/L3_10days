#include "EnemyStation.h"

#include "imgui.h"

EnemyStation::EnemyStation(){}

EnemyStation::EnemyStation(const std::string& name):
	BaseStation(name){}

/// ===================================================
/// 初期化
/// ===================================================
void EnemyStation::Init(){
	BaseStation::Init();

}

/// ===================================================
/// 更新
/// ===================================================
void EnemyStation::Update(){ BaseStation::Update(); }
