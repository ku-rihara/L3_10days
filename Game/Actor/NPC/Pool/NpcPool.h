#pragma once

#include <vector>
#include <memory>
#include <cassert>

class NPC;

class NpcPoolBase {
public:
	virtual ~NpcPoolBase() = default;
	virtual void Release(NPC* p)noexcept = 0;
};

struct NpcDeleter {
	NpcPoolBase* pool{};
	void operator()(NPC* p) const noexcept { if (pool && p) pool->Release(p); }
};

using NpcHandle = std::unique_ptr<NPC, NpcDeleter>;

// 型付きプール
template<class T>
class NpcPool : public NpcPoolBase {
public:
	// 事前確保
	void Prewarm(size_t n) {
		for (size_t i = 0; i < n; ++i) {
			store_.push_back(std::make_unique<T>());
			free_.push_back(store_.back().get());
		}
	}

	// 借用
	NpcHandle Acquire() {
		T* obj = nullptr;
		if (!free_.empty()) {
			obj = free_.back();
			free_.pop_back();
		} else {
			store_.push_back(std::make_unique<T>());
			obj = store_.back().get();
		}
		obj->Activate(); // 再利用時の有効化
		return NpcHandle(obj, NpcDeleter{ this });
	}

	// 返却（deleter から呼ばれる）
	void Release(NPC* p) noexcept override {
		auto* obj = static_cast<T*>(p);
		obj->Deactivate();     // 無効化
		free_.push_back(obj);  // フリーリストへ
	}

private:
	std::vector<std::unique_ptr<T>> store_;	// 実体の所有者
	std::vector<T*> free_;					// 再利用待ち
};