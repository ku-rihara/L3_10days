#include "Dissolve.h"
#include "base/TextureManager.h"
#include "Dx/DirectXCommon.h"
#include "Dx/DxRenderTarget.h"
#include "Function/Log.h"
#include <cassert>
#include <imgui.h>

void Dissolve::Init(DirectXCommon* dxCommon) {

    vsName_ = L"resources/Shader/PostEffect/Fullscreen.VS.hlsl";
    psName_ = L"resources/Shader/PostEffect/Dissolve.PS.hlsl";
    BasePostEffect::Init(dxCommon);

    textureIndex_ = TextureManager::GetInstance()->LoadTexture("Resources/EngineTexture/noise0.png");
}

void Dissolve::CreateGraphicsPipeline() {
    BasePostEffect::CreateGraphicsPipeline();
}

void Dissolve::CreateRootSignature() {
    HRESULT hr = 0;
    // RootSignatureを作成
    D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
    descriptionRootSignature.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

    D3D12_DESCRIPTOR_RANGE descriptorRange[2]            = {};
    descriptorRange[0].RangeType                         = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    descriptorRange[0].NumDescriptors                    = 1;
    descriptorRange[0].BaseShaderRegister                = 0; // t0
    descriptorRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    descriptorRange[1].RangeType                         = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    descriptorRange[1].NumDescriptors                    = 1;
    descriptorRange[1].BaseShaderRegister                = 1; // t1
    descriptorRange[1].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    // ルートパラメータ
    D3D12_ROOT_PARAMETER rootParameters[3] = {};

    // color
    rootParameters[0].ParameterType                       = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParameters[0].ShaderVisibility                    = D3D12_SHADER_VISIBILITY_PIXEL;
    rootParameters[0].DescriptorTable.pDescriptorRanges   = &descriptorRange[0];
    rootParameters[0].DescriptorTable.NumDescriptorRanges = 1;

    // Masktexture
    rootParameters[1].ParameterType                       = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParameters[1].ShaderVisibility                    = D3D12_SHADER_VISIBILITY_PIXEL;
    rootParameters[1].DescriptorTable.pDescriptorRanges   = &descriptorRange[1];
    rootParameters[1].DescriptorTable.NumDescriptorRanges = 1;

    // 定数バッファ
    rootParameters[2].ParameterType             = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParameters[2].ShaderVisibility          = D3D12_SHADER_VISIBILITY_PIXEL;
    rootParameters[2].Descriptor.ShaderRegister = 0;

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

void Dissolve::SetDrawState(ID3D12GraphicsCommandList* commandList) {
    BasePostEffect::SetDrawState(commandList);
}

void Dissolve::CreateConstantBuffer() {
    D3D12_RANGE readRange = {};
    HRESULT hr;

    // param (b0)
    paramDataResource_ = dxCommon_->CreateBufferResource(dxCommon_->GetDevice(), sizeof(ParamData));
    hr                 = paramDataResource_->Map(0, &readRange, reinterpret_cast<void**>(&paramData_));
    if (FAILED(hr)) {
        // エラー処理
        OutputDebugStringA("ConstBuffer Map failed.\n");
    }

    paramData_->thresholdValue = 0.0f;
    paramData_->color          = {1.0f, 0.4f, 0.3f};
}
void Dissolve::Draw([[maybe_unused]] ID3D12GraphicsCommandList* commandList) {

    // プリミティブトポロジーを設定
    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    // テクスチャリソースを設定
    commandList->SetGraphicsRootDescriptorTable(0, dxCommon_->GetDxRenderTarget()->GetRenderTextureGPUSrvHandle());
    // t1
    commandList->SetGraphicsRootDescriptorTable(1, TextureManager::GetInstance()->GetTextureHandle(textureIndex_));
    // b0: param
    commandList->SetGraphicsRootConstantBufferView(2, paramDataResource_->GetGPUVirtualAddress());

    commandList->DrawInstanced(3, 1, 0, 0);
}

void Dissolve::DebugParamImGui() {
#ifdef _DEBUG
    if (ImGui::CollapsingHeader("Dissolve")) {
        ImGui::DragFloat("thresholdValue", &paramData_->thresholdValue, 0.01f);
        ImGui::ColorEdit3("color", &paramData_->color.x);
    }
#endif // _DEBUG
}