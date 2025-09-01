#include "IndexBuffer.h"

IndexBuffer::IndexBuffer() {}
IndexBuffer::~IndexBuffer() {}

void IndexBuffer::Create(size_t _indicesSize, DxDevice* _dxDevice) {
	Resize(_indicesSize);

	{	/// bufferの生成
		HRESULT result = S_FALSE;

		/// ヒープ設定
		D3D12_HEAP_PROPERTIES uploadHeapProperties{};
		uploadHeapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;

		D3D12_RESOURCE_DESC desc{};
		desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER; /// バッファリソース
		desc.Width = sizeof(uint32_t) * indices_.size();                          /// リソースのサイズ
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

		//Assert(SUCCEEDED(result), "Resource creation failed.");
	}

	ibv_.BufferLocation = resource_.Get()->GetGPUVirtualAddress();
	ibv_.SizeInBytes = static_cast<UINT>(sizeof(uint32_t) * indices_.size());
	ibv_.Format = DXGI_FORMAT_R32_UINT;
}

void IndexBuffer::Reserve(size_t _value) {
	indices_.reserve(_value);
}

void IndexBuffer::Resize(size_t _value) {
	indices_.resize(_value);
}

void IndexBuffer::BindForCommandList(ID3D12GraphicsCommandList* _commandList) {
	_commandList->IASetIndexBuffer(&ibv_);
}

void IndexBuffer::Map() {
	uint32_t* map = nullptr;
	resource_.Get()->Map(0, nullptr, reinterpret_cast<void**>(&map));
	std::memcpy(map, indices_.data(), sizeof(uint32_t) * indices_.size());
}

const std::vector<uint32_t>& IndexBuffer::GetIndices() const {
	return indices_;
}

void IndexBuffer::SetIndex(size_t _indexIndex, uint32_t _indexValue) {
	indices_[_indexIndex] = _indexValue;
}

void IndexBuffer::SetIndices(const std::vector<uint32_t>& _indices) {
	indices_ = _indices;
}
