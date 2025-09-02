#pragma once

/// directX
#include <d3dcommon.h>
#include <dxcapi.h>
#include <wrl/client.h>

/// engine
#include "Material/ModelMaterial.h"
#include "Dx/DirectXCommon.h"
#include "3d/ViewProjection.h"
#include "struct/TransformationMatrix.h"
#include "3d/Model.h"

/// buffer
#include "Buffer/StructuredBuffer.h"
#include "Buffer/VertexBuffer.h"
#include "Buffer/IndexBuffer.h"


/// //////////////////////////////////////////////////////
/// 境界の破片の描画パイプライン
/// //////////////////////////////////////////////////////
class BoundaryShardPipeline {

	enum ROOT_PARAM {
		ROOT_PARAM_TRANSFORM,
		ROOT_PARAM_SHADOW_TRANSFORM,
		ROOT_PARAM_INSTANCE_COUNT,
		ROOT_PARAM_ROOT_CONSTANT,
	};

public:
	/// ========================================================
	/// public : methods
	/// ========================================================

	BoundaryShardPipeline()  = default;
	~BoundaryShardPipeline() = default;

	/// @brief シングルトン化
	static BoundaryShardPipeline* GetInstance();

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

};

