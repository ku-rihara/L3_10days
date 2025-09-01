#pragma once
#include"struct/Transform.h"
#include"Matrix4x4.h"
#include<string>
#include<cstdint>
#include<optional>
#include<map>
#include<vector>

struct Joint {
    QuaternionTransform transform;
    Matrix4x4 localMatrix;
    Matrix4x4 skeletonSpaceMatrix;
    std::string name;
    std::vector<int32_t> children;
    int32_t index;
    std::optional<int32_t> parent;
};

struct Skeleton {
    int32_t root;
    std::map<std::string, int32_t> jointMap;
    std::vector<Joint> joints;
};