#pragma once

/// directX
#include <d3dcommon.h>
#include <dxcapi.h>
#include <wrl/client.h>

/// engine
#include "Material/ModelMaterial.h"
#include "Dx/DirectXCommon.h"

/// ///////////////////////////////////////////////////////
/// ミニマップ描画パイプライン
/// ///////////////////////////////////////////////////////
class MiniMapIconPipeline {
	MiniMapIconPipeline() = default;
	~MiniMapIconPipeline() = default;
public:
	/// ========================================================
	/// public : methods
	/// ========================================================

	/// @brief シングルトン化
	static MiniMapIconPipeline* GetInstance();

	void Init(DirectXCommon* _dxCommon);
	void PreDraw(ID3D12GraphicsCommandList* _cmdList);
	void Draw(ID3D12GraphicsCommandList* _cmdList);

private:
	/// ========================================================
	/// private : methods
	/// ========================================================

	void CreateRootSignature();
	void CreateGraphicsPipeline();

private:
	/// ========================================================
	/// private : objects
	/// ========================================================
	DirectXCommon* dxCommon_;

	D3D12_STATIC_SAMPLER_DESC staticSamplers_;
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_;
	Microsoft::WRL::ComPtr<ID3DBlob> signatureBlob_;
	Microsoft::WRL::ComPtr<ID3DBlob> errorBlob_;
	Microsoft::WRL::ComPtr<IDxcBlob> vertexShaderBlob_;
	Microsoft::WRL::ComPtr<IDxcBlob> pixelShaderBlob_;

	Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPipelineStateNone_;
	D3D12_DEPTH_STENCIL_DESC depthStencilDesc_;


};

