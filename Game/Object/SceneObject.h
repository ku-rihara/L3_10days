#pragma once

#include "BaseObject/BaseObject.h"

class SceneObject : public BaseObject {
public:
	SceneObject(const std::string& _path);
	~SceneObject() override;
	void Init() override;
	void Update() override;
	Object3d* GetObj3d();
private:
	std::string path_;
};

