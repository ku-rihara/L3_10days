#pragma once

/// directX
#include <d3dcommon.h>
#include <dxcapi.h>
#include <wrl/client.h>

/// engine
#include "Material/ModelMaterial.h"
#include "Dx/DirectXCommon.h"
#include "Vector2.h"

#include "Pipeline/Buffer/VertexBuffer.h"
#include "Pipeline/Buffer/IndexBuffer.h"

struct IconVertex {
	Vector4 position; // xyz座標
	Vector2 uv;       // uv座標
};

/// ///////////////////////////////////////////////////////
/// ミニマップ描画パイプライン
/// ///////////////////////////////////////////////////////
class MiniMapIconPipeline {
	MiniMapIconPipeline() = default;
	~MiniMapIconPipeline() = default;

	enum ROOT_PARAM {
		ROOT_PARAM_ICON,
		ROOT_PARAM_MINI_MAP_SIZE,
		ROOT_PARAM_TEXTURE,
	};

public:
	/// ========================================================
	/// public : methods
	/// ========================================================

	/// @brief シングルトン化
	static MiniMapIconPipeline* GetInstance();

	void Init(DirectXCommon* _dxCommon);
	void PreDraw(ID3D12GraphicsCommandList* _cmdList);
	void Draw(ID3D12GraphicsCommandList* _cmdList, class MiniMap* _miniMap);

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


	VertexBuffer<IconVertex> vertexBuffer_;
	IndexBuffer indexBuffer_;

};

