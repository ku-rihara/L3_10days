#include "BoundaryShardPipeline.h"

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

/// 描画対象
#include "../Actor/Boundary/Boundary.h"

BoundaryShardPipeline* BoundaryShardPipeline::GetInstance() {
	static BoundaryShardPipeline instance;
	return &instance;
}

void BoundaryShardPipeline::Init(DirectXCommon* dxCommon) {

	// 引数で受けとる
	dxCommon_ = dxCommon;
	// グラフィックスパイプラインの生成
	CreateGraphicsPipeline();
}

void BoundaryShardPipeline::CreateGraphicsPipeline() {

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

	// InputLayoutの設定を行う
	D3D12_INPUT_ELEMENT_DESC inputElementDescs[3] = {};
	inputElementDescs[0].SemanticName = "POSITION";
	inputElementDescs[0].SemanticIndex = 0;
	inputElementDescs[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	inputElementDescs[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

	inputElementDescs[1].SemanticName = "TEXCOORD";
	inputElementDescs[1].SemanticIndex = 0;
	inputElementDescs[1].Format = DXGI_FORMAT_R32G32_FLOAT;
	inputElementDescs[1].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

	inputElementDescs[2].SemanticName = "NORMAL";
	inputElementDescs[2].SemanticIndex = 0;
	inputElementDescs[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	inputElementDescs[2].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};
	inputLayoutDesc.pInputElementDescs = inputElementDescs;
	inputLayoutDesc.NumElements = _countof(inputElementDescs);

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
	rasterizerDesc.CullMode = D3D12_CULL_MODE_NONE;
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
	vertexShaderBlob_ = dxCommon_->GetDxCompiler()->CompileShader(L"resources/Shader/BoundaryShard.vs.hlsl", L"vs_6_0");
	pixelShaderBlob_ = dxCommon_->GetDxCompiler()->CompileShader(L"resources/Shader/BoundaryShard.ps.hlsl", L"ps_6_0");

	// PSO作成用関数
	auto CreatePSO = [&](D3D12_BLEND_DESC& blendDesc, Microsoft::WRL::ComPtr<ID3D12PipelineState>& pso) {
		D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDesc = {};
		graphicsPipelineStateDesc.pRootSignature = rootSignature_.Get();
		graphicsPipelineStateDesc.InputLayout = inputLayoutDesc;
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

void BoundaryShardPipeline::CreateRootSignature() {
	HRESULT hr = 0;
	// RootSignatureを作成
	D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
	descriptionRootSignature.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	descriptionRootSignature.pStaticSamplers = staticSamplers_;
	descriptionRootSignature.NumStaticSamplers = 2; // 通常サンプラーとシャドウサンプラーの2個

	// DescriptorRangeの設定
	D3D12_DESCRIPTOR_RANGE descriptorRange[6] = {};

	// holes (t0)
	descriptorRange[0].BaseShaderRegister = 0;
	descriptorRange[0].NumDescriptors = 1;
	descriptorRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	descriptorRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	// 環境テクスチャ (t1)
	descriptorRange[1].BaseShaderRegister = 1;
	descriptorRange[1].NumDescriptors = 1;
	descriptorRange[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	descriptorRange[1].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	// ポイントライト (t2)
	descriptorRange[2].BaseShaderRegister = 2;
	descriptorRange[2].NumDescriptors = 1;
	descriptorRange[2].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	descriptorRange[2].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	// スポットライト (t3)
	descriptorRange[3].BaseShaderRegister = 3;
	descriptorRange[3].NumDescriptors = 1;
	descriptorRange[3].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	descriptorRange[3].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	// シャドウマップ (t4)
	descriptorRange[4].BaseShaderRegister = 4;
	descriptorRange[4].NumDescriptors = 1;
	descriptorRange[4].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	descriptorRange[4].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	// dissolve (t5)
	descriptorRange[5].BaseShaderRegister = 5;
	descriptorRange[5].NumDescriptors = 1;
	descriptorRange[5].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	descriptorRange[5].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	// RootParameterを作成
	D3D12_ROOT_PARAMETER rootParameters[4] = {};

	// 0: TransformationMatrix
	rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE; // CBVを使う
	rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX; // VertexShaderを使う
	rootParameters[0].Descriptor.ShaderRegister = 0; // レジスタ番号0とバインド
	rootParameters[0].DescriptorTable.pDescriptorRanges = &descriptorRange[0];
	rootParameters[0].DescriptorTable.NumDescriptorRanges = 1;

	// 1: ShadowMap
	rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
	rootParameters[1].Descriptor.ShaderRegister = 0;

	// 2: InstanceCount	
	rootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
	rootParameters[2].Descriptor.ShaderRegister = 1;

	// 3: InstanceCount	
	rootParameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
	rootParameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
	rootParameters[3].Constants.Num32BitValues = 1;
	rootParameters[3].Constants.ShaderRegister = 2;


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

void BoundaryShardPipeline::PreDraw(ID3D12GraphicsCommandList* commandList) {
	// RootSignatureを設定
	commandList->SetGraphicsRootSignature(rootSignature_.Get());
	// パイプラインステートの設定
	commandList->SetPipelineState(graphicsPipelineStateNone_.Get());
	/// プリミティブ形状を設定
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void BoundaryShardPipeline::Draw(ID3D12GraphicsCommandList* commandList, const ViewProjection& _viewProjection) {
	Boundary* boundary = Boundary::GetInstance();
	if (!boundary) { return; }

	BoundaryShard* boundaryShard = boundary->GetBoundaryShard();
	if (!boundaryShard) { return; }

	const auto& breakables = boundary->GetBreakables();
	UINT instanceCount = static_cast<UINT>(breakables.size());

	/// 罅がなかったら描画しない
	if (instanceCount == 0) { return; }

	/// vertex shader buffers
	auto& shadowBuf = boundary->GetShadowTransformBufferRef();
	shadowBuf.SetMappedData({ ShadowMap::GetInstance()->GetLightCameraMatrix() });
	shadowBuf.BindForGraphicsCommandList(commandList, ROOT_PARAM_SHADOW_TRANSFORM);

	/// 各罅の行列を計算してbufferに詰める
	Matrix4x4 matWorld, matWVP, matWorldInverseTranspose;
	for (size_t i = 0; i < breakables.size(); i++) {
		const Breakable& breakable = breakables[i];
		for (size_t j = 0; j < breakable.shards.size(); j++) {
			const Shard& shard = breakable.shards[j];

			matWorld = MakeIdentity4x4();
			matWorld *= MakeScaleMatrix({ breakable.radius, breakable.radius, breakable.radius });
			matWorld *= MakeRotateMatrix(shard.transform.rotate);
			matWorld *= MakeTranslateMatrix(shard.transform.translate * breakable.radius);
			matWorld *= MakeTranslateMatrix(breakable.position);

			matWVP = matWorld * _viewProjection.matView_ * _viewProjection.matProjection_;
			matWorldInverseTranspose = Transpose(Inverse(matWorld));

			size_t index = i * breakable.shards.size() + j;
			boundaryShard->breakableTransformBuffer_.SetMappedData(index, { matWVP, matWorld, matWorldInverseTranspose });
		}
	}

	boundaryShard->breakableTransformBuffer_.BindToCommandList(ROOT_PARAM_TRANSFORM, commandList);

	//instanceCount *= static_cast<UINT>(boundaryShard->loadedShards_.size());
	int shardCount = static_cast<int>(boundaryShard->GetLoadedShards().size());
	boundaryShard->instanceCountBuffer_.SetMappedData(shardCount);
	boundaryShard->instanceCountBuffer_.BindForGraphicsCommandList(commandList, ROOT_PARAM_INSTANCE_COUNT);



	/// 描画
	for (size_t i = 0; i < boundaryShard->GetLoadedShards().size(); i++) {
		/// 破片モデルをセット
		const Shard& shardModel = boundaryShard->GetLoadedShards()[i];
		shardModel.vertexBuffer.BindForCommandList(commandList);
		shardModel.indexBuffer.BindForCommandList(commandList);

		commandList->SetGraphicsRoot32BitConstants(
			ROOT_PARAM_ROOT_CONSTANT, 1, &i, 0
		);

		UINT indexCount = static_cast<UINT>(shardModel.indexBuffer.GetIndices().size());
		commandList->DrawIndexedInstanced(indexCount, instanceCount, 0, 0, 0);
	}
}