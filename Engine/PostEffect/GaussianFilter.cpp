#include "Dx/DirectXCommon.h"
#include"Dx/DxRenderTarget.h"
#include "GaussianFilter.h"
#include "function/Log.h"
#include <cassert>
#include <d3dx12.h>
#include <imgui.h>

void GaussianFilter::Init(DirectXCommon* dxCommon) {

    vsName_ = L"resources/Shader/PostEffect/Fullscreen.VS.hlsl";
    psName_ = L"resources/Shader/PostEffect/GaussianFilter.PS.hlsl";
    BasePostEffect::Init(dxCommon);
}

void GaussianFilter::CreateGraphicsPipeline() {
    BasePostEffect::CreateGraphicsPipeline();
}
void GaussianFilter::CreateRootSignature() {
    HRESULT hr = 0;
    // RootSignatureを作成
    D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
    descriptionRootSignature.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

    D3D12_DESCRIPTOR_RANGE descriptorRangeColor[1]            = {};
    descriptorRangeColor[0].RangeType                         = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    descriptorRangeColor[0].NumDescriptors                    = 1;
    descriptorRangeColor[0].BaseShaderRegister                = 0; // t0
    descriptorRangeColor[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    // ルートパラメータ
    D3D12_ROOT_PARAMETER rootParameters[2] = {};

    // color
    rootParameters[0].ParameterType                       = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParameters[0].ShaderVisibility                    = D3D12_SHADER_VISIBILITY_PIXEL;
    rootParameters[0].DescriptorTable.pDescriptorRanges   = descriptorRangeColor;
    rootParameters[0].DescriptorTable.NumDescriptorRanges = _countof(descriptorRangeColor);

    // 定数バッファ
    rootParameters[1].ParameterType             = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParameters[1].ShaderVisibility          = D3D12_SHADER_VISIBILITY_PIXEL;
    rootParameters[1].Descriptor.ShaderRegister = 0;

    descriptionRootSignature.pParameters       = rootParameters;
    descriptionRootSignature.NumParameters     = _countof(rootParameters);
    descriptionRootSignature.pStaticSamplers   = staticSamplers_;
    descriptionRootSignature.NumStaticSamplers = _countof(staticSamplers_);

    hr = D3D12SerializeRootSignature(&descriptionRootSignature, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob_, &errorBlob_);
    if (FAILED(hr)) {
        Log(reinterpret_cast<char*>(errorBlob_->GetBufferPointer()));
        assert(false);
    }

    hr = dxCommon_->GetDevice()->CreateRootSignature(0, signatureBlob_->GetBufferPointer(), signatureBlob_->GetBufferSize(), IID_PPV_ARGS(&rootSignature_));
    assert(SUCCEEDED(hr));
}

void GaussianFilter::SetDrawState(ID3D12GraphicsCommandList* commandList) {
    BasePostEffect::SetDrawState(commandList);
}

void GaussianFilter::CreateConstantBuffer() {
    D3D12_RANGE readRange = {};
    HRESULT hr;

    // param (b0)
    paramDataResource_ = dxCommon_->CreateBufferResource(dxCommon_->GetDevice(), sizeof(ParamData));
    hr                 = paramDataResource_->Map(0, &readRange, reinterpret_cast<void**>(&paramData_));
    if (FAILED(hr)) {
        // エラー処理
        OutputDebugStringA("ConstBuffer Map failed.\n");
    }

    paramData_->sigma = 2.0f;
}
void GaussianFilter::Draw([[maybe_unused]] ID3D12GraphicsCommandList* commandList) {

    // プリミティブトポロジーを設定
    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    // テクスチャリソースを設定
    commandList->SetGraphicsRootDescriptorTable(0, dxCommon_->GetDxRenderTarget()->GetRenderTextureGPUSrvHandle());

    // b0:param
    commandList->SetGraphicsRootConstantBufferView(1, paramDataResource_->GetGPUVirtualAddress());

    commandList->DrawInstanced(3, 1, 0, 0);
}

void GaussianFilter::DebugParamImGui() {
#ifdef _DEBUG
    if (ImGui::CollapsingHeader("GaussianFilter")) {
        ImGui::DragFloat("sigma", &paramData_->sigma, 0.01f);
    }
#endif // _DEBUG
}