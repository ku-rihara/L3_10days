#pragma once
#include <vector>

class BaseObject;

struct ITargetProvider {
    virtual ~ITargetProvider() = default;
    virtual void CollectTargets(std::vector<const BaseObject*>& out) const = 0;
};