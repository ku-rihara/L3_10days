#pragma once
#include <vector>
#include <memory>

class NpcBullet;

class NpcBulletPoolBase {
public:
	virtual ~NpcBulletPoolBase() = default;
	virtual void Release(NpcBullet* bullet) noexcept = 0;
};

struct NpcBulletDeleter {
	NpcBulletPoolBase* pool{};
	void operator()(NpcBullet* bullet) const noexcept {
		if (pool && bullet) pool->Release(bullet);
	}
};

using NpcBulletHandle = std::unique_ptr<NpcBullet, NpcBulletDeleter>;

template<class T>
class NpcBulletPool : public NpcBulletPoolBase {
public:
	void Prewarm(size_t n);
	NpcBulletHandle Acquire();
	void Release(NpcBullet* bullet) noexcept override;

private:
	std::vector<std::unique_ptr<T>> store_;
	std::vector<T*> free_;
};

template<class T>
inline void NpcBulletPool<T>::Prewarm(size_t n) {
	for (size_t i = 0; i < n; ++i) {
		store_.push_back(std::make_unique<T>());
		free_.push_back(store_.back().get());
	}
}

template<class T>
inline NpcBulletHandle NpcBulletPool<T>::Acquire() {
	T* obj = nullptr;
	if (!free_.empty()) {
		obj = free_.back();
		free_.pop_back();
	} else {
		store_.push_back(std::make_unique<T>());
		obj = store_.back().get();
	}
	NpcBulletDeleter del; del.pool = this;
	return NpcBulletHandle(obj, del);
}

template<class T>
inline void NpcBulletPool<T>::Release(NpcBullet* p) noexcept {
	auto* obj = static_cast<T*>(p);
	obj->Deactivate();
	free_.push_back(obj);
}
