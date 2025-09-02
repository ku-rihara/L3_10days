#pragma once

/// directX
#include <d3d12.h>
#include <wrl/client.h>

/// std
#include <vector>

#include "Dx/DxDevice.h"


using Microsoft::WRL::ComPtr;


/// ///////////////////////////////////////////////////
/// index buffer view
/// ///////////////////////////////////////////////////
template<typename T>
class VertexBuffer final {
public:
	/// ===================================================
	/// public : methods
	/// ===================================================

	VertexBuffer() = default;
	~VertexBuffer() = default;

	void Create(size_t _indicesSize, class DxDevice* _dxDevice);

	void Reserve(size_t _value);
	void Resize(size_t _value);

	void BindForCommandList(ID3D12GraphicsCommandList* _commandList);

	void Map();

private:
	/// ===================================================
	/// private : objects
	/// ===================================================

	ComPtr<ID3D12Resource> resource_;
	std::vector<T> vertices_;
	D3D12_VERTEX_BUFFER_VIEW vbv_;
	T* mappingData_ = nullptr;

public:
	/// ===================================================
	/// public : accessor
	/// ===================================================

	const std::vector<T>& GetVertices() const;

	void SetVertex(size_t _index, const T& _vertex);
	void SetVertices(const std::vector<T>& _vertices);

};



template<typename T>
inline void VertexBuffer<T>::Create(size_t _vertexSize, DxDevice* _dxDevice) {
	size_t tSize = sizeof(T);

	Resize(_vertexSize);

	{	/// bufferの生成
		HRESULT result = S_FALSE;

		/// ヒープ設定
		D3D12_HEAP_PROPERTIES uploadHeapProperties{};
		uploadHeapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;

		D3D12_RESOURCE_DESC desc{};
		desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER; /// バッファリソース
		desc.Width = tSize * vertices_.size();            /// リソースのサイズ
		desc.Height = 1;
		desc.DepthOrArraySize = 1;
		desc.MipLevels = 1;
		desc.SampleDesc.Count = 1;
		desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

		/// リソースの作成
		result = _dxDevice->GetDevice()->CreateCommittedResource(
			&uploadHeapProperties,
			D3D12_HEAP_FLAG_NONE,
			&desc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&resource_)
		);
	}

	vbv_.BufferLocation = resource_.Get()->GetGPUVirtualAddress();
	vbv_.SizeInBytes = static_cast<UINT>(tSize * vertices_.size());
	vbv_.StrideInBytes = static_cast<UINT>(tSize);

}

template<typename T>
inline void VertexBuffer<T>::Reserve(size_t _value) {
	vertices_.reserve(_value);
}

template<typename T>
inline void VertexBuffer<T>::Resize(size_t _value) {
	vertices_.resize(_value);
}

template<typename T>
inline void VertexBuffer<T>::BindForCommandList(ID3D12GraphicsCommandList* _commandList) {
	_commandList->IASetVertexBuffers(0, 1, &vbv_);
}

template<typename T>
inline void VertexBuffer<T>::Map() {
	resource_.Get()->Map(0, nullptr, reinterpret_cast<void**>(&mappingData_));
	std::memcpy(mappingData_, vertices_.data(), sizeof(T) * vertices_.size());
}

template<typename T>
inline const std::vector<T>& VertexBuffer<T>::GetVertices() const {
	return vertices_;
}

template<typename T>
inline void VertexBuffer<T>::SetVertex(size_t _index, const T& _vertex) {
	if (mappingData_) {
		mappingData_[_index] = _vertex;
	}
	vertices_[_index] = _vertex;
}

template<typename T>
inline void VertexBuffer<T>::SetVertices(const std::vector<T>& _vertices) {
	vertices_ = _vertices;
}
