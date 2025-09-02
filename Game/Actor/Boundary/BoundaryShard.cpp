#include "BoundaryShard.h"

/// game
#include "Boundary.h"

BoundaryShard::BoundaryShard() {}
BoundaryShard::~BoundaryShard() {}

void BoundaryShard::Init() {

}

void BoundaryShard::Update() {

	/// 境界のポインタを取得
	Boundary* boundary = Boundary::GetInstance();
	if (!boundary) {
		return;
	}

	/// 境界の罅の数を取得
	auto& cracks = boundary->GetCracksRef();
	if(cracks.size() == 0) {
		return;
	}

	/// 罅の数だけ破片を生成
	for(auto itr = cracks.begin(); itr != cracks.end(); ++itr) {
		//const Crack& crack = *itr;
	}

}

void BoundaryShard::BindVBVAndIBV(ID3D12GraphicsCommandList* _cmdList, size_t _shardIndex) {
	Shard& shard = shards_[_shardIndex];
	shard.vertexBuffer.BindForCommandList(_cmdList);
	shard.indexBuffer.BindForCommandList(_cmdList);
}
