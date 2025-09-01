#pragma once

///// engine
//#include "Engine/Core/DirectX12/Resource/DxResource.h"
//#include "Engine/Core/DirectX12/Device/DxDevice.h"

/// directX
#include <d3d12.h>

/// engine
#include "Dx/DxDevice.h"

using Microsoft::WRL::ComPtr;

/// ===================================================
/// 定数バッファクラス
/// ===================================================
template <typename T>
class ConstantBuffer final {
public:

	/// ===================================================
	/// public : methods
	/// ===================================================

	ConstantBuffer() {}
	~ConstantBuffer() {}

	/// @brief バッファの生成
	/// @param _dxDevice DxDeviceへのポインタ
	void Create(DxDevice* _dxDevice);

	/// @brief graphics pipeline にバインド
	/// @param _commandList ID3D12GraphicsCommandList
	/// @param _rootParameterIndex root parameter index
	void BindForGraphicsCommandList(ID3D12GraphicsCommandList* _commandList, UINT _rootParameterIndex);

	/// @brief compute pipeline にバインド
	/// @param _commandList ID3D12GraphicsCommandList
	/// @param _rootParameterIndex root parameter index
	void BindForComputeCommandList(ID3D12GraphicsCommandList* _commandList, UINT _rootParameterIndex);

private:

	/// ===================================================
	/// private : objects
	/// ===================================================

	ComPtr<ID3D12Resource> constantBuffer_;
	T*                     mappingData_;


public:

	/// ===================================================
	/// public : accessor
	/// ===================================================

	/// @brief mappingDataの設定
	/// @param _mappingData 設定するデータ
	void SetMappedData(const T& _mappingData);

	/// @brief mappingDataの取得
	/// @return 取得したデータ
	const T& GetMappingData() const { return *mappingData_; }

	/// @brief Resourceの取得
	/// @return リソースへのポインタ
	ID3D12Resource* Get() const { return constantBuffer_.Get(); }

};


template<typename T>
inline void ConstantBuffer<T>::Create(DxDevice* _dxDevice) {

	{
		HRESULT result = S_FALSE;

		/// ヒープ設定
		D3D12_HEAP_PROPERTIES uploadHeapProperties{};
		uploadHeapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;

		D3D12_RESOURCE_DESC desc{};
		desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER; /// バッファリソース
		desc.Width = sizeof(T);            /// リソースのサイズ
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
			IID_PPV_ARGS(&constantBuffer_)
		);
	}

	mappingData_ = nullptr;
	constantBuffer_.Get()->Map(0, nullptr, reinterpret_cast<void**>(&mappingData_));
	SetMappedData(T{}); ///< 0クリア
}

template<typename T>
inline void ConstantBuffer<T>::BindForGraphicsCommandList(ID3D12GraphicsCommandList* _commandList, UINT _rootParameterIndex) {
	_commandList->SetGraphicsRootConstantBufferView(_rootParameterIndex, constantBuffer_.Get()->GetGPUVirtualAddress());
}

template<typename T>
inline void ConstantBuffer<T>::BindForComputeCommandList(ID3D12GraphicsCommandList* _commandList, UINT _rootParameterIndex) {
	_commandList->SetComputeRootConstantBufferView(_rootParameterIndex, constantBuffer_.Get()->GetGPUVirtualAddress());
}

template<typename T>
inline void ConstantBuffer<T>::SetMappedData(const T& _mappingData) {
	*mappingData_ = _mappingData;
}
