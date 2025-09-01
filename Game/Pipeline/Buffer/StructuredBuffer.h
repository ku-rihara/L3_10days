#pragma once

/// directX12
#include <d3d12.h>
#include <wrl/client.h>

/// std
#include <cstdint>
#include <span>

/// engine
//#include "Engine/Core/DirectX12/ComPtr/ComPtr.h"
//#include "Engine/Core/DirectX12/Resource/DxResource.h"
//#include "Engine/Core/DirectX12/Device/DxDevice.h"
//#include "Engine/Core/DirectX12/DescriptorHeap/DxSRVHeap.h"
//#include "Engine/Core/Utility/Tools/Assert.h"

#include "Dx/DxDevice.h"
#include "base/SrvManager.h"


using Microsoft::WRL::ComPtr;

/// ===================================================
/// ストラクチャードバッファ用クラス
/// ===================================================
template <typename T>
class StructuredBuffer final {
public:

	/// ===================================================
	/// public : methods
	/// ===================================================

	StructuredBuffer();
	~StructuredBuffer();

	void Create(uint32_t _size, DxDevice* _dxDevice);
	//void CreateUAV(uint32_t _size, DxDevice* _dxDevice, DxCommand* _dxCommand);

	void BindToCommandList(UINT _rootParameterIndex, ID3D12GraphicsCommandList* _commandList);
	void BindForComputeCommandList(UINT _rootParameterIndex, ID3D12GraphicsCommandList* _commandList);

	void SetMappedData(size_t _index, const T& _setValue);

	ID3D12Resource* GetResource();

private:

	/// ===================================================
	/// private : objects
	/// ===================================================

	uint32_t                    srvDescriptorIndex_;
	D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle_;
	D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle_;

	ComPtr<ID3D12Resource>      bufferResource_;
	T*                          mappedData_;
	std::span<T>                mappedDataArray_;

	size_t                      structureSize_;
	size_t                      totalSize_;
	size_t                      bufferSize_;
};


/// ===================================================
/// public : method definition
/// ===================================================

template<typename T>
inline StructuredBuffer<T>::StructuredBuffer() {}

template<typename T>
inline StructuredBuffer<T>::~StructuredBuffer() {}


template<typename T>
inline void StructuredBuffer<T>::Create(uint32_t _size, DxDevice* _dxDevice) {

	/// bufferのサイズを計算
	structureSize_ = sizeof(T);
	bufferSize_ = _size;
	totalSize_ = structureSize_ * bufferSize_;

	{	/// bufferの生成
		HRESULT result = S_FALSE;

		/// ヒープ設定
		D3D12_HEAP_PROPERTIES uploadHeapProperties{};
		uploadHeapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;

		D3D12_RESOURCE_DESC desc{};
		desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER; /// バッファリソース
		desc.Width = totalSize_;                          /// リソースのサイズ
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
			IID_PPV_ARGS(&bufferResource_)
		);

		//Assert(SUCCEEDED(result), "Resource creation failed.");
	}


	/// desc setting
	D3D12_SHADER_RESOURCE_VIEW_DESC desc{};
	desc.Format = DXGI_FORMAT_UNKNOWN;
	desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	desc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	desc.Buffer.FirstElement = 0;
	desc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
	desc.Buffer.NumElements = static_cast<UINT>(bufferSize_);
	desc.Buffer.StructureByteStride = static_cast<UINT>(structureSize_);

	/// cpu, gpu handle initialize
	SrvManager* pSRVManager = SrvManager::GetInstance();
	srvDescriptorIndex_ = pSRVManager->Allocate();
	cpuHandle_ = pSRVManager->GetCPUDescriptorHandle(srvDescriptorIndex_);
	gpuHandle_ = pSRVManager->GetGPUDescriptorHandle(srvDescriptorIndex_);

	/// resource create
	_dxDevice->GetDevice()->CreateShaderResourceView(bufferResource_.Get(), &desc, cpuHandle_);

	/// mapping
	bufferResource_.Get()->Map(0, nullptr, reinterpret_cast<void**>(&mappedData_));
	mappedDataArray_ = { mappedData_, bufferSize_ };
}
//
//template<typename T>
//inline void StructuredBuffer<T>::CreateUAV(uint32_t _size, DxDevice* _dxDevice, DxCommand* _dxCommand) {
//
//	/// bufferのサイズを計算
//	structureSize_ = sizeof(T);
//	bufferSize_ = _size;
//	totalSize_ = structureSize_ * bufferSize_;
//
//	/// bufferの生成
//	{
//		HRESULT result = S_FALSE;
//
//		CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_DEFAULT);
//		CD3DX12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Buffer(
//			_sizeInByte,
//			D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS
//		);
//
//
//		// 1. Create on DEFAULT heap
//		_dxDevice->GetDevice()->CreateCommittedResource(
//			&heapProps,
//			D3D12_HEAP_FLAG_NONE,
//			&desc,
//			D3D12_RESOURCE_STATE_COMMON, // ← 初期状態は素直にCOMMON
//			nullptr,
//			IID_PPV_ARGS(&resource_)
//		);
//
//		// 2. Barrier to UNORDERED_ACCESS before dispatch
//		CD3DX12_RESOURCE_BARRIER uavBarrier = CD3DX12_RESOURCE_BARRIER::Transition(
//			resource_.Get(),
//			D3D12_RESOURCE_STATE_COMMON,
//			D3D12_RESOURCE_STATE_UNORDERED_ACCESS
//		);
//
//		_dxCommand->GetCommandList()->ResourceBarrier(1, &uavBarrier);
//
//		//Assert(SUCCEEDED(result), "UAV Resource creation failed.");
//	}
//
//	/// desc setting
//	D3D12_UNORDERED_ACCESS_VIEW_DESC  desc{};
//	desc.Format = DXGI_FORMAT_UNKNOWN;
//	desc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
//	desc.Buffer.FirstElement = 0;
//	desc.Buffer.NumElements = static_cast<UINT>(bufferSize_);
//	desc.Buffer.StructureByteStride = static_cast<UINT>(structureSize_);
//	desc.Buffer.CounterOffsetInBytes = 0;
//	desc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;
//
//	/// cpu, gpu handle initialize
//	srvDescriptorIndex_ = pDxSRVHeap_->AllocateBuffer();
//	cpuHandle_ = pDxSRVHeap_->GetCPUDescriptorHandel(srvDescriptorIndex_);
//	gpuHandle_ = pDxSRVHeap_->GetGPUDescriptorHandel(srvDescriptorIndex_);
//
//	/// resource create
//	_dxDevice->GetDevice()->CreateUnorderedAccessView(bufferResource_.Get(), nullptr, &desc, cpuHandle_);
//
//}


template<typename T>
inline void StructuredBuffer<T>::BindToCommandList(UINT _rootParameterIndex, ID3D12GraphicsCommandList* _commandList) {
	_commandList->SetGraphicsRootDescriptorTable(_rootParameterIndex, gpuHandle_);
}

template<typename T>
inline void StructuredBuffer<T>::BindForComputeCommandList(UINT _rootParameterIndex, ID3D12GraphicsCommandList* _commandList) {
	_commandList->SetComputeRootDescriptorTable(_rootParameterIndex, gpuHandle_);
}

template<typename T>
inline void StructuredBuffer<T>::SetMappedData(size_t _index, const T& _setValue) {
	//Assert(_index < bufferSize_, "out of range");
	mappedDataArray_[_index] = _setValue;
}

template<typename T>
inline ID3D12Resource* StructuredBuffer<T>::GetResource() {
	return bufferResource_;
}
