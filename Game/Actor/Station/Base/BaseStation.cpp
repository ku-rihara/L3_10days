#include "BaseStation.h"

void BaseStation::Init(){
	obj3d_.reset(Object3d::CreateModel("cube.obj"));
	BaseObject::Init();
}

void BaseStation::Update(){ BaseObject::Update(); }
