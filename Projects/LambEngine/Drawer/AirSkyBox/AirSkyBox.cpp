/// =============================
/// ==  AirSkyBoxクラスの定義  ==
/// =============================

#include "AirSkyBox.h"
#include <array>
#include "Math/MathCommon.h"
#include "Engine/Core/DescriptorHeap/CbvSrvUavHeap.h"
#include "Engine/Core/DirectXCommand/DirectXCommand.h"
#include "Engine/Graphics/TextureManager/TextureManager.h"
#include "Engine/Graphics/PipelineManager/PipelineManager.h"

#include "Engine/Graphics/Shader/ShaderManager/ShaderManager.h"

AirSkyBox::~AirSkyBox()
{
}

void AirSkyBox::Load() {
    const std::array vertexData = {
        Vector4(1.0f,  1.0f, -1.0f, 1.0f),
        Vector4(1.0f, -1.0f, -1.0f, 1.0f),
        Vector4(1.0f,  1.0f,  1.0f, 1.0f),
        Vector4(1.0f, -1.0f,  1.0f, 1.0f),
        Vector4(-1.0f,  1.0f, -1.0f, 1.0f),
        Vector4(-1.0f, -1.0f, -1.0f, 1.0f),
        Vector4(-1.0f,  1.0f,  1.0f, 1.0f),
        Vector4(-1.0f, -1.0f,  1.0f, 1.0f),
    };

    const std::array indexData = {
        0_u16, 2_u16, 4_u16,
        3_u16, 7_u16, 2_u16,
        7_u16, 5_u16, 6_u16,
        5_u16, 7_u16, 1_u16,
        1_u16, 3_u16, 0_u16,
        5_u16, 1_u16, 4_u16,
        2_u16, 6_u16, 4_u16,
        7_u16, 6_u16, 2_u16,
        5_u16, 4_u16, 6_u16,
        7_u16, 3_u16, 1_u16,
        3_u16, 2_u16, 0_u16,
        1_u16, 0_u16, 4_u16,
    };

    Lamb::SafePtr directXDevice = DirectXDevice::GetInstance();


    uint32_t indexSizeInBytes = static_cast<uint32_t>(sizeof(uint16_t) * indexData.size());
    uint32_t vertexSizeInBytes = static_cast<uint32_t>(sizeof(Vector4) * vertexData.size());


    // インデックス
    pIndexResource_ = directXDevice->CreateBufferResuorce(indexSizeInBytes);

    Lamb::SafePtr<uint16_t> indexMap = nullptr;
    pIndexResource_->Map(0, nullptr, indexMap.GetPtrAdress());
    std::copy(indexData.begin(), indexData.end(), indexMap);
    pIndexResource_->Unmap(0, nullptr);

    indexView_.SizeInBytes = indexSizeInBytes;
    indexView_.Format = DXGI_FORMAT_R16_UINT;
    indexView_.BufferLocation = pIndexResource_->GetGPUVirtualAddress();

    // 頂点
    pVertexResource_ = directXDevice->CreateBufferResuorce(vertexSizeInBytes);

    Lamb::SafePtr<Vector4> vertMap = nullptr;
    pVertexResource_->Map(0, nullptr, vertMap.GetPtrAdress());
    std::copy(vertexData.begin(), vertexData.end(), vertMap);
    pVertexResource_->Unmap(0, nullptr);

    vertexView_.SizeInBytes = vertexSizeInBytes;
    vertexView_.StrideInBytes = static_cast<uint32_t>(sizeof(Vector4));
    vertexView_.BufferLocation = pVertexResource_->GetGPUVirtualAddress();

    // バッファ作成
    std::for_each(shaderData_.begin(), shaderData_.end(), [](auto& n) {
        n = std::make_unique<ConstantBuffer<ShaderData>>();
        });
    std::for_each(atmosphericParams_.begin(), atmosphericParams_.end(), [](auto& n) {
        n = std::make_unique<ConstantBuffer<AtmosphericParams>>();
        });

    // パイプライン作成
    CreateGraphicsPipeline();
}

void AirSkyBox::Draw(const Mat4x4& worldMat, const Mat4x4& cameraMat, uint32_t color) {
    shaderData_[Lamb::GetGraphicBufferIndex()]->Map();
    (*shaderData_[Lamb::GetGraphicBufferIndex()])->worldMat = worldMat;
    (*shaderData_[Lamb::GetGraphicBufferIndex()])->viewProjectionMat = cameraMat;
    (*shaderData_[Lamb::GetGraphicBufferIndex()])->color = color;
    shaderData_[Lamb::GetGraphicBufferIndex()]->Unmap();

    // コマンドリスト
    ID3D12GraphicsCommandList* const commandlist = DirectXCommand::GetMainCommandlist()->GetCommandList();

    // パイプライン設定
    pPipeline_->Use();

    // ライト構造体
    commandlist->SetGraphicsRootConstantBufferView(0, shaderData_[Lamb::GetGraphicBufferIndex()]->GetGPUVtlAdrs());
    commandlist->SetGraphicsRootConstantBufferView(1, atmosphericParams_[Lamb::GetGraphicBufferIndex()]->GetGPUVtlAdrs());

    // 頂点バッファセット
    commandlist->IASetVertexBuffers(0, 1, &vertexView_);
    // インデックスバッファセット
    commandlist->IASetIndexBuffer(&indexView_);
    // ドローコール
    commandlist->DrawIndexedInstanced(kIndexNumber_, 1, 0, 0, 0);
}

void AirSkyBox::SetAtmosphericParams(const AtmosphericParams& atmosphericParams)
{
    atmosphericParams_[Lamb::GetGraphicBufferIndex()]->MemCpy(&atmosphericParams);
}

void AirSkyBox::CreateGraphicsPipeline() {
    Shader shader = {};

    ShaderManager* const shaderMaanger = ShaderManager::GetInstance();


    shader.vertex = shaderMaanger->LoadVertexShader("./Shaders/SkyBoxShader/SkyBox.VS.hlsl");
    shader.pixel = shaderMaanger->LoadPixelShader("./Shaders/SkyBoxShader/AirSkyBox.PS.hlsl");


    std::array<D3D12_ROOT_PARAMETER, 2> rootPrams = {};
    rootPrams[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootPrams[0].Descriptor.ShaderRegister = 0;
    rootPrams[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

    rootPrams[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootPrams[1].Descriptor.ShaderRegister = 1;
    rootPrams[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;


    RootSignature::Desc desc;
    desc.rootParameter = rootPrams.data();
    desc.rootParameterSize = rootPrams.size();
    desc.samplerDeacs.push_back(
        CreateLinearSampler()
    );

    auto pipelineManager = PipelineManager::GetInstance();
    Pipeline::Desc pipelineDesc;
    pipelineDesc.rootSignature = pipelineManager->CreateRootSgnature(desc);
    pipelineDesc.vsInputData.push_back({ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT });
    pipelineDesc.shader = shader;
    pipelineDesc.isDepth = true;
    pipelineDesc.blend[0] = Pipeline::None;
    pipelineDesc.rtvFormtat[0] = DXGI_FORMAT_R32G32B32A32_FLOAT;
    pipelineDesc.solidState = Pipeline::SolidState::Solid;
    pipelineDesc.cullMode = Pipeline::CullMode::Back;
    pipelineDesc.topologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    pipelineDesc.numRenderTarget = 1;
    pipelineManager->SetDesc(pipelineDesc);
    pPipeline_ = pipelineManager->CreateCubeMap();
    pipelineManager->StateReset();
}
