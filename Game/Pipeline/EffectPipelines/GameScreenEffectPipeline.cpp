#include "GameScreenEffectPipeline.h"

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


GameScreenEffectPipeline* GameScreenEffectPipeline::GetInstance() {
	static GameScreenEffectPipeline instance;
	return &instance;
}

void GameScreenEffectPipeline::Init(DirectXCommon* dxCommon) {

	// 引数で受けとる
	dxCommon_ = dxCommon;
	// グラフィックスパイプラインの生成
	CreateGraphicsPipeline();

	timeBuffer_.Create(DirectXCommon::GetInstance()->GetDxDevice());
	timeBuffer_.SetMappedData(0);

	effectBuffer_.Create(DirectXCommon::GetInstance()->GetDxDevice());
	effectBuffer_.SetMappedData({});
}

void GameScreenEffectPipeline::CreateGraphicsPipeline() {

	HRESULT hr = 0;

	// 通常のサンプラー
	staticSamplers_[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR; // バイリニアフィルタ
	staticSamplers_[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP; // 0~1の範囲外をリピート
	staticSamplers_[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers_[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers_[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER; // 比較しない
	staticSamplers_[0].MaxLOD = D3D12_FLOAT32_MAX; // ありったけのMipMapを使う
	staticSamplers_[0].ShaderRegister = 0; // レジスタ番号０
	staticSamplers_[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL; // pxelShaderで使う

	// シャドウマップ用比較サンプラー
	staticSamplers_[1].Filter = D3D12_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
	staticSamplers_[1].AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	staticSamplers_[1].AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	staticSamplers_[1].AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	staticSamplers_[1].BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE;
	staticSamplers_[1].ComparisonFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
	staticSamplers_[1].MaxLOD = D3D12_FLOAT32_MAX;
	staticSamplers_[1].ShaderRegister = 1; // レジスタ番号1
	staticSamplers_[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	staticSamplers_[1].MaxAnisotropy = 1;

	CreateRootSignature();

	// BlendMode None
	D3D12_BLEND_DESC blendDescNormal = {};
	blendDescNormal.RenderTarget[0].BlendEnable = TRUE;
	blendDescNormal.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
	blendDescNormal.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
	blendDescNormal.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
	blendDescNormal.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
	blendDescNormal.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
	blendDescNormal.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
	blendDescNormal.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	// RasterizerStateの設定
	D3D12_RASTERIZER_DESC rasterizerDesc{};
	// 裏面を表示しない
	rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
	// 三角形の色を塗りつぶす
	rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;

	// DepthStencilStateの設定-------------------------------------
	// Depthの機能を有効化する
	depthStencilDesc_.DepthEnable = true;
	// 書き込みする
	depthStencilDesc_.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	// 比較関数はLessEqual。つまり、近ければ描画される
	depthStencilDesc_.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;

	// Shaderをコンパイルする
	vertexShaderBlob_ = dxCommon_->GetDxCompiler()->CompileShader(L"resources/Shader/GameScreenEffect.vs.hlsl", L"vs_6_0");
	pixelShaderBlob_ = dxCommon_->GetDxCompiler()->CompileShader(L"resources/Shader/GameScreenEffect.ps.hlsl", L"ps_6_0");

	// PSO作成用関数
	auto CreatePSO = [&](D3D12_BLEND_DESC& blendDesc, Microsoft::WRL::ComPtr<ID3D12PipelineState>& pso) {
		D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDesc = {};
		graphicsPipelineStateDesc.pRootSignature = rootSignature_.Get();
		//graphicsPipelineStateDesc.InputLayout = inputLayoutDesc;
		graphicsPipelineStateDesc.VS = { vertexShaderBlob_->GetBufferPointer(), vertexShaderBlob_->GetBufferSize() };
		graphicsPipelineStateDesc.PS = { pixelShaderBlob_->GetBufferPointer(), pixelShaderBlob_->GetBufferSize() };
		graphicsPipelineStateDesc.BlendState = blendDesc;
		graphicsPipelineStateDesc.RasterizerState = rasterizerDesc;
		graphicsPipelineStateDesc.NumRenderTargets = 1;
		graphicsPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
		graphicsPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		graphicsPipelineStateDesc.SampleDesc.Count = 1;
		graphicsPipelineStateDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
		graphicsPipelineStateDesc.DepthStencilState = depthStencilDesc_;
		graphicsPipelineStateDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

		hr = dxCommon_->GetDevice()->CreateGraphicsPipelineState(&graphicsPipelineStateDesc, IID_PPV_ARGS(&pso));
		assert(SUCCEEDED(hr));
		};

	// 各ブレンドモードのPSOを作成
	CreatePSO(blendDescNormal, graphicsPipelineStateNone_);
}

void GameScreenEffectPipeline::CreateRootSignature() {
	HRESULT hr = 0;
	// RootSignatureを作成
	D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
	descriptionRootSignature.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	descriptionRootSignature.pStaticSamplers = staticSamplers_;
	descriptionRootSignature.NumStaticSamplers = 2; // 通常サンプラーとシャドウサンプラーの2個

	// RootParameterを作成
	D3D12_ROOT_PARAMETER rootParameters[2] = {};

	// 0: Time
	rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameters[0].Descriptor.ShaderRegister = 0;

	// 1: EffectBufData
	rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameters[1].Descriptor.ShaderRegister = 1;

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

void GameScreenEffectPipeline::PreDraw(ID3D12GraphicsCommandList* commandList) {
	// RootSignatureを設定
	commandList->SetGraphicsRootSignature(rootSignature_.Get());
	// パイプラインステートの設定
	commandList->SetPipelineState(graphicsPipelineStateNone_.Get());
	/// プリミティブ形状を設定
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void GameScreenEffectPipeline::Draw(ID3D12GraphicsCommandList* _cmdList, GameScreenEffect* _playerOutsideWarning) {
	if (!_playerOutsideWarning) {
		return;
	}

	timeBuffer_.SetMappedData(timeBuffer_.GetMappingData() + Frame::DeltaTime());
	timeBuffer_.BindForGraphicsCommandList(_cmdList, ROOT_PARAM_TIME);

	effectBuffer_.SetMappedData(_playerOutsideWarning->GetBaseColor());
	effectBuffer_.BindForGraphicsCommandList(_cmdList, ROOT_PARAM_EFFECT_BUFFER_DATA);

	/// vertexは
	_cmdList->DrawIndexedInstanced(
		6, 1, 0, 0, 0
	);
}
