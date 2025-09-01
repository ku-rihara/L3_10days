#pragma once

/// directX
#include <d3dcommon.h>
#include <dxcapi.h>
#include <wrl/client.h>

/// engine
#include "Material/ModelMaterial.h"
#include "Dx/DirectXCommon.h"
#include "3d/ViewProjection.h"

/// //////////////////////////////////////////////////////
/// ボーダーの描画パイプライン
/// //////////////////////////////////////////////////////
class BoundaryPipeline {
public:

	enum ROOT_PARAM {
		ROOT_PARAM_TRANSFORM,
		ROOT_PARAM_SHADOW_TRANSFORM,
		ROOT_PARAM_HOLE,
		ROOT_PARAM_TIME,
	};

public:
	/// ========================================================
	/// public : methods
	/// ========================================================

	BoundaryPipeline() = default;
	~BoundaryPipeline() = default;

	/// @brief シングルトン化
	static BoundaryPipeline* GetInstance();

	void Init(DirectXCommon* dxCommon);
	void PreDraw(ID3D12GraphicsCommandList* commandList);
	void Draw(ID3D12GraphicsCommandList* commandList, const ViewProjection& _viewProjection);

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

	D3D12_STATIC_SAMPLER_DESC staticSamplers_[2];
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_;
	Microsoft::WRL::ComPtr<ID3DBlob> signatureBlob_;
	Microsoft::WRL::ComPtr<ID3DBlob> errorBlob_;
	Microsoft::WRL::ComPtr<IDxcBlob> vertexShaderBlob_;
	Microsoft::WRL::ComPtr<IDxcBlob> pixelShaderBlob_;

	Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPipelineStateNone_;
	D3D12_DEPTH_STENCIL_DESC depthStencilDesc_;


public:
	// getter
	DirectXCommon* GetDxCommon() const { return dxCommon_; }

	// rootSignature
	ID3D12RootSignature* GetRootSignature() const { return rootSignature_.Get(); }
};

