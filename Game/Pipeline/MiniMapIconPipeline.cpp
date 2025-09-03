#include "MiniMapIconPipeline.h"

/// std
#include <cassert>
#include <string>
#include <numbers>

/// engine
#include "Dx/DxCompiler.h"
#include "function/Log.h"
#include "3d/ModelManager.h"
#include "random.h"
#include "Frame/Frame.h"
#include "base/TextureManager.h"

/// game
#include "Actor/MiniMap/MiniMap.h"


MiniMapIconPipeline* MiniMapIconPipeline::GetInstance() {
	static MiniMapIconPipeline instance;
	return &instance;
}

void MiniMapIconPipeline::Init(DirectXCommon* _dxCommon) {
	// 引数で受けとる
	dxCommon_ = _dxCommon;
	// グラフィックスパイプラインの生成
	CreateGraphicsPipeline();

	/// vbv ibv
	vertexBuffer_.Create(4, DirectXCommon::GetInstance()->GetDxDevice());
	indexBuffer_.Create(6, DirectXCommon::GetInstance()->GetDxDevice());

	vertexBuffer_.SetVertices({
		{ { -0.5f, +0.5f, 0.0f, 1.0f }, { 0.0f, 0.0f } },
		{ { +0.5f, +0.5f, 0.0f, 1.0f }, { 1.0f, 0.0f } },
		{ { -0.5f, -0.5f, 0.0f, 1.0f }, { 0.0f, 1.0f } },
		{ { +0.5f, -0.5f, 0.0f, 1.0f }, { 1.0f, 1.0f } }
		});

	indexBuffer_.SetIndices({ 0, 1, 2, 1, 3, 2 });

	vertexBuffer_.Map();
	indexBuffer_.Map();

}

void MiniMapIconPipeline::CreateGraphicsPipeline() {
	HRESULT hr = 0;

	// 通常のサンプラー
	staticSamplers_.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR; // バイリニアフィルタ
	staticSamplers_.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP; // 0~1の範囲外をリピート
	staticSamplers_.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers_.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers_.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER; // 比較しない
	staticSamplers_.MaxLOD = D3D12_FLOAT32_MAX; // ありったけのMipMapを使う
	staticSamplers_.ShaderRegister = 0; // レジスタ番号０
	staticSamplers_.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL; // pxelShaderで使う

	CreateRootSignature();

	// InputLayoutの設定を行う
	D3D12_INPUT_ELEMENT_DESC inputElementDescs[2] = {};
	inputElementDescs[0].SemanticName = "POSITION";
	inputElementDescs[0].SemanticIndex = 0;
	inputElementDescs[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	inputElementDescs[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

	inputElementDescs[1].SemanticName = "TEXCOORD";
	inputElementDescs[1].SemanticIndex = 0;
	inputElementDescs[1].Format = DXGI_FORMAT_R32G32_FLOAT;
	inputElementDescs[1].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};
	inputLayoutDesc.pInputElementDescs = inputElementDescs;
	inputLayoutDesc.NumElements = _countof(inputElementDescs);

	// BlendMode None
	D3D12_BLEND_DESC blendDescNormal = {};
	blendDescNormal.RenderTarget[0].BlendEnable = TRUE;
	blendDescNormal.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
	blendDescNormal.RenderTarget[0].DestBlend = D3D12_BLEND_ONE;
	blendDescNormal.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
	blendDescNormal.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
	blendDescNormal.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ONE;
	blendDescNormal.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
	blendDescNormal.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	// RasterizerStateの設定
	D3D12_RASTERIZER_DESC rasterizerDesc{};
	// 裏面を表示しない
	rasterizerDesc.CullMode = D3D12_CULL_MODE_NONE;
	// 三角形の色を塗りつぶす
	rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;

	// DepthStencilStateの設定-------------------------------------
	// Depthの機能を有効化する
	depthStencilDesc_.DepthEnable = false;
	// 書き込みする
	depthStencilDesc_.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	// 比較関数はLessEqual。つまり、近ければ描画される
	depthStencilDesc_.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;

	// Shaderをコンパイルする
	vertexShaderBlob_ = dxCommon_->GetDxCompiler()->CompileShader(L"resources/Shader/MiniMapIcon.vs.hlsl", L"vs_6_0");
	pixelShaderBlob_ = dxCommon_->GetDxCompiler()->CompileShader(L"resources/Shader/MiniMapIcon.ps.hlsl", L"ps_6_0");

	// PSO作成用関数
	D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDesc = {};
	graphicsPipelineStateDesc.pRootSignature = rootSignature_.Get();
	graphicsPipelineStateDesc.InputLayout = inputLayoutDesc;
	graphicsPipelineStateDesc.VS = { vertexShaderBlob_->GetBufferPointer(), vertexShaderBlob_->GetBufferSize() };
	graphicsPipelineStateDesc.PS = { pixelShaderBlob_->GetBufferPointer(), pixelShaderBlob_->GetBufferSize() };
	graphicsPipelineStateDesc.BlendState = blendDescNormal;
	graphicsPipelineStateDesc.RasterizerState = rasterizerDesc;
	graphicsPipelineStateDesc.NumRenderTargets = 1;
	graphicsPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	graphicsPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	graphicsPipelineStateDesc.SampleDesc.Count = 1;
	graphicsPipelineStateDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
	graphicsPipelineStateDesc.DepthStencilState = depthStencilDesc_;
	graphicsPipelineStateDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

	hr = dxCommon_->GetDevice()->CreateGraphicsPipelineState(&graphicsPipelineStateDesc, IID_PPV_ARGS(&graphicsPipelineStateNone_));
	assert(SUCCEEDED(hr));

	// 各ブレンドモードのPSOを作成
	//CreatePSO(blendDescNormal, graphicsPipelineStateNone_);
}

void MiniMapIconPipeline::CreateRootSignature() {
	HRESULT hr = 0;
	// RootSignatureを作成
	D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
	descriptionRootSignature.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	descriptionRootSignature.pStaticSamplers = &staticSamplers_;
	descriptionRootSignature.NumStaticSamplers = 1; // 通常サンプラーの1個

	// DescriptorRangeの設定
	D3D12_DESCRIPTOR_RANGE descriptorRange[2] = {};

	// holes (t0)
	descriptorRange[0].BaseShaderRegister = 1;
	descriptorRange[0].NumDescriptors = 1;
	descriptorRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	descriptorRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	// テクスチャ (t1)
	descriptorRange[1].BaseShaderRegister = 0;
	descriptorRange[1].NumDescriptors = 1;
	descriptorRange[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	descriptorRange[1].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	// RootParameterを作成
	D3D12_ROOT_PARAMETER rootParameters[3] = {};

	// 0: gIcons
	rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE; // CBVを使う
	rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX; // VertexShaderを使う
	rootParameters[0].DescriptorTable.pDescriptorRanges = &descriptorRange[1];
	rootParameters[0].DescriptorTable.NumDescriptorRanges = 1;

	// 2: Texture
	rootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameters[2].DescriptorTable.pDescriptorRanges = &descriptorRange[0];
	rootParameters[2].DescriptorTable.NumDescriptorRanges = 1;

	// 1: MiniMapSize
	rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameters[1].Descriptor.ShaderRegister = 0;

	descriptionRootSignature.pParameters = rootParameters; // ルートパラメーターの配列
	descriptionRootSignature.NumParameters = _countof(rootParameters); // 配列の長さ

	// シリアライズしてバイナリにする
	signatureBlob_ = nullptr;
	errorBlob_ = nullptr;
	hr = D3D12SerializeRootSignature(&descriptionRootSignature, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob_, &errorBlob_);
	if (FAILED(hr)) {
		Log(reinterpret_cast<char*>(errorBlob_->GetBufferPointer()));
		assert(false);
	}

	// バイナリを元に生成
	rootSignature_ = nullptr;
	hr = dxCommon_->GetDevice()->CreateRootSignature(0, signatureBlob_->GetBufferPointer(), signatureBlob_->GetBufferSize(), IID_PPV_ARGS(&rootSignature_));
	assert(SUCCEEDED(hr));
}

void MiniMapIconPipeline::PreDraw(ID3D12GraphicsCommandList* _cmdList) {
	// パイプラインステートの設定
	_cmdList->SetPipelineState(graphicsPipelineStateNone_.Get());
	// RootSignatureを設定
	_cmdList->SetGraphicsRootSignature(rootSignature_.Get());
	/// プリミティブ形状を設定
	_cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void MiniMapIconPipeline::Draw(ID3D12GraphicsCommandList* _cmdList, MiniMap* _miniMap) {
	if (!_miniMap) {
		return;
	}



	vertexBuffer_.BindForCommandList(_cmdList);
	indexBuffer_.BindForCommandList(_cmdList);


	const size_t kIconCount = 2;
	StructuredBuffer<IconBufferData>* iconBuffer[kIconCount] = {
		&_miniMap->GetFriendIconBuffer(),
		&_miniMap->GetEnemyIconBuffer()
	};

	UINT instanceCounts[kIconCount] = {
		_miniMap->GetFriendIconCount(),
		_miniMap->GetEnemyIconCount()
	};

	/// MiniMapSize Bind
	_miniMap->GetMiniMapSizeBuffer().BindForGraphicsCommandList(_cmdList, ROOT_PARAM_MINI_MAP_SIZE);

	/// Texture Bind
	TextureManager* textureManager = TextureManager::GetInstance();
	uint32_t textureIndex = textureManager->LoadTexture("./resources/Texture/MiniMap/Icon.png");
	D3D12_GPU_DESCRIPTOR_HANDLE	gpuHandle = textureManager->GetTextureHandle(textureIndex);
	_cmdList->SetGraphicsRootDescriptorTable(ROOT_PARAM_TEXTURE, gpuHandle);

	UINT indexCount = static_cast<UINT>(indexBuffer_.GetIndices().size());
	for (size_t i = 0; i < kIconCount; i++) {
		iconBuffer[i]->BindToCommandList(ROOT_PARAM_ICON, _cmdList);

		_cmdList->DrawIndexedInstanced(indexCount, instanceCounts[i], 0, 0, 0);
	}
}