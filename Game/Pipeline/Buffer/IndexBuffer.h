#pragma once

/// std
#include <vector>

#include <d3d12.h>
#include <wrl/client.h>

#include "Dx/DxDevice.h"

using Microsoft::WRL::ComPtr;

/// ///////////////////////////////////////////////////
/// index buffer view
/// ///////////////////////////////////////////////////
class IndexBuffer final {
public:
	/// ===================================================
	/// public : methods
	/// ===================================================

	IndexBuffer();
	~IndexBuffer();

	void Create(size_t _indicesSize, class DxDevice* _dxDevice);

	void Reserve(size_t _value);
	void Resize(size_t _value);

	void BindForCommandList(ID3D12GraphicsCommandList* _commandList) const;

	void Map();

private:
	/// ===================================================
	/// private : objects
	/// ===================================================

	ComPtr<ID3D12Resource> resource_;
	std::vector<uint32_t> indices_;
	D3D12_INDEX_BUFFER_VIEW ibv_;


public:
	/// ===================================================
	/// public : accessor
	/// ===================================================

	const std::vector<uint32_t>& GetIndices() const;

	void SetIndex(size_t _indexIndex, uint32_t _indexValue);
	void SetIndices(const std::vector<uint32_t>& _indices);

};

