#include "SceneObject.h"

SceneObject::SceneObject(const std::string& _path)
	: path_(_path) {
	BaseObject::Init();
	obj3d_.reset(Object3d::CreateModel(path_));
	obj3d_->transform_.SetParent(&baseTransform_);
}

SceneObject::~SceneObject() = default;

void SceneObject::Init() {}

void SceneObject::Update() {
	BaseObject::Update();
}

Object3d* SceneObject::GetObj3d() {
	return obj3d_.get();
}