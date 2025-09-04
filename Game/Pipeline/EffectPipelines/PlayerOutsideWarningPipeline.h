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

#include "Pipeline/Buffer/ConstantBuffer.h"

/// 描画対象
#include "Actor/Effects/PlayerOutsideWarning/PlayerOutsideWarning.h"

class PlayerOutsideWarningPipeline {

	enum ROOT_PARAM {
		ROOT_PARAM_TIME,
	};

public:
	/// ========================================================
	/// public : methods
	/// ========================================================

	PlayerOutsideWarningPipeline() = default;
	~PlayerOutsideWarningPipeline() = default;

	/// @brief シングルトン化
	static PlayerOutsideWarningPipeline* GetInstance();

	void Init(DirectXCommon* dxCommon);
	void PreDraw(ID3D12GraphicsCommandList* commandList);
	void Draw(
		ID3D12GraphicsCommandList* _cmdList,
		PlayerOutsideWarning* _playerOutsideWarning
	);

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

	ConstantBuffer<float> timeBuffer_;

};

