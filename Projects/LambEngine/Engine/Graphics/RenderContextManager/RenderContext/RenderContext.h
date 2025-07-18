/// =================================
/// ==  RenderContextクラスの定義  ==
/// =================================


#pragma once
#include "../../GraphicsStructs.h"
#include "../../Meshlet/Meshlet.h"
#include "Engine/Core/DescriptorHeap/CbvSrvUavHeap.h"
#include "Engine/Core/DirectXCommand/DirectXCommand.h"

#include "Engine/Graphics/Skeleton.h"

#include "Utils/EngineInfo.h"

#include <typeinfo>
#include <concepts>
#include <array>
#include <memory>
#include <algorithm>


class BaseRenderContext {
public:
    static constexpr uint32_t kMaxDrawInstance = 256u;


public:
    BaseRenderContext():
        vertexIndexData_(nullptr),
        modelData_(nullptr),
        pipeline_(nullptr),
        drawCount_(0),
        shadowPipeline_(nullptr),
        lightCamera_(),
        typeID_()
    {
        std::for_each(
            lightCamera_.begin(),
            lightCamera_.end(),
            [](auto& n) {
                n = std::make_unique<ConstantBuffer<float32_t4x4>>();
            }
        );
    }
    virtual ~BaseRenderContext() = default;

    virtual void Draw() const = 0;
    virtual void DrawShadow() const = 0;

    const std::string& GetID() const {
        return typeID_;
    }

/// <summary>
/// セッター
/// </summary>
public:
    virtual void SetVertexIndexData(const VertexIndexData* const) = 0;
    virtual void SetModelData(const ModelData* const modelData) = 0;
    virtual void SetPipeline(Pipeline* const pipeline) = 0;
    virtual void SetWVPMatrix(const WVPMatrix& matrix) = 0;
    virtual void SetColor(const Vector4& color) = 0;
    virtual void SetLight(const DirectionLight& light) = 0;
    virtual void SetCameraPos(const Vector3& cameraPos) = 0;

    void SetShadowPipeline(Pipeline* const shadowPipeline) {
        if (not shadowPipeline_ and shadowPipeline) {
            shadowPipeline_ = shadowPipeline;
        }
    }

    void SetLightCameraMatrix(const float32_t4x4& lightCameraMatrix) {
        lightCamera_[Lamb::GetGraphicBufferIndex()]->MemCpy(lightCameraMatrix.data());
    }
public:
    /// <summary>
    /// ポジションベースのzソート
    /// </summary>
    virtual void ZSort() = 0;
    /// <summary>
    /// バッファにデータをセットする
    /// </summary>
    virtual void SetData() = 0;


public:
    void AddDrawCount() {
        drawCount_++;
    }

    void ResetDrawCount() {
        drawCount_ = 0u;
    }

    bool IsDraw() const {
        return drawCount_ != 0u;
    }

    const std::string& GetRootName() const {
        return vertexIndexData_->node.name;
    }

    const VertexIndexData* const GetVertexIndexData() const {
        return vertexIndexData_;
    }

    const ModelData* const GetModelData() const {
        return modelData_;
    }

    const Node& GetNode() const {
        return vertexIndexData_->node;
    }

protected:
    const VertexIndexData* vertexIndexData_;
    const ModelData* modelData_;

    Pipeline* pipeline_;
    uint32_t drawCount_;

    Pipeline* shadowPipeline_;
    std::array<std::unique_ptr<ConstantBuffer<float32_t4x4>>, DirectXSwapChain::kBackBufferNumber> lightCamera_;

    std::string typeID_;
};

using RenderData = BaseRenderContext;


template<class T = uint32_t, uint32_t bufferSize = RenderData::kMaxDrawInstance>
class RenderContext : public BaseRenderContext {
public:
    RenderContext():
        BaseRenderContext(),
        shaderData_()
    {
        for (uint32_t i = 0; i < DirectXSwapChain::kBackBufferNumber; ++i) {
            shaderData_[i].wvpMatrix.CreateBuffer(bufferSize);
            shaderData_[i].color.CreateBuffer(bufferSize);
            shaderData_[i].shaderStruct.CreateBuffer(bufferSize);
        }

        drawData_.resize(bufferSize);


        pipeline_ = nullptr;
        drawCount_ = 0u;
       

        typeID_ = (typeid(RenderContext<T, bufferSize>).name());
    }
    ~RenderContext() = default;

    RenderContext(const RenderContext&) = delete;
    RenderContext(RenderContext&&) = delete;

    RenderContext& operator=(const RenderContext&) = delete;
    RenderContext& operator=(RenderContext&&) = delete;

public:
    void Draw() const override {
        // ディスクリプタヒープ
        CbvSrvUavHeap* const descriptorHeap = CbvSrvUavHeap::GetInstance();
        // コマンドリスト
        ID3D12GraphicsCommandList* const commandlist = DirectXCommand::GetMainCommandlist()->GetCommandList();

        // パイプライン設定
        pipeline_->Use();

        // ライト構造体
        commandlist->SetGraphicsRootConstantBufferView(0, shaderData_[Lamb::GetGraphicBufferIndex()].light.GetGPUVtlAdrs());
        // カメラポジション
        commandlist->SetGraphicsRootConstantBufferView(1, shaderData_[Lamb::GetGraphicBufferIndex()].eyePos.GetGPUVtlAdrs());
        // ワールドとカメラマトリックス
        commandlist->SetGraphicsRootShaderResourceView(2, shaderData_[Lamb::GetGraphicBufferIndex()].wvpMatrix.GetGPUVtlAdrs());
        // 色
        commandlist->SetGraphicsRootShaderResourceView(3, shaderData_[Lamb::GetGraphicBufferIndex()].color.GetGPUVtlAdrs());
        // shaderの固有構造体
        commandlist->SetGraphicsRootShaderResourceView(4, shaderData_[Lamb::GetGraphicBufferIndex()].shaderStruct.GetGPUVtlAdrs());
        // テクスチャ
        commandlist->SetGraphicsRootDescriptorTable(5, descriptorHeap->GetGpuHeapHandle(0));

        // 頂点バッファセット
        commandlist->IASetVertexBuffers(0, 1, &(vertexIndexData_->vertexView));
        // インデックスバッファセット
        commandlist->IASetIndexBuffer(&(vertexIndexData_->indexView));
        // ドローコール
        commandlist->DrawIndexedInstanced(vertexIndexData_->indexNumber, drawCount_, 0, 0, 0);
    }

    void DrawShadow() const override {
        // コマンドリスト
        ID3D12GraphicsCommandList* const commandlist = DirectXCommand::GetMainCommandlist()->GetCommandList();

        // パイプライン設定
        shadowPipeline_->Use();

        // ライトのカメラマトリックス
        commandlist->SetGraphicsRootConstantBufferView(0, lightCamera_[Lamb::GetGraphicBufferIndex()]->GetGPUVtlAdrs());
        // ワールドとカメラマトリックス
        commandlist->SetGraphicsRootShaderResourceView(1, shaderData_[Lamb::GetGraphicBufferIndex()].wvpMatrix.GetGPUVtlAdrs());

        // 頂点バッファセット
        commandlist->IASetVertexBuffers(0, 1, &(vertexIndexData_->vertexView));
        // インデックスバッファセット
        commandlist->IASetIndexBuffer(&(vertexIndexData_->indexView));
        // ドローコール
        commandlist->DrawIndexedInstanced(vertexIndexData_->indexNumber, drawCount_, 0, 0, 0);
    }
    
public:
    inline void SetVertexIndexData(const VertexIndexData* const vertexIndexData) override {
        if (!vertexIndexData) {
            throw Lamb::Error::Code<RenderContext>("vertexIndexData is nullptr", ErrorPlace);
        }
        vertexIndexData_ = vertexIndexData;
    }
    inline void SetModelData(const ModelData* const modelData) override {
        if (!modelData) {
            throw Lamb::Error::Code<RenderContext>("modelData is nullptr", ErrorPlace);
        }
        modelData_ = modelData;
    }
    inline void SetPipeline(Pipeline* const pipeline) override {
        if (!pipeline) {
            throw Lamb::Error::Code<RenderContext>("pipeline is nullptr", ErrorPlace);
        }
        pipeline_ = pipeline;
    }
    inline void SetWVPMatrix(const WVPMatrix& matrix) override {
        if (bufferSize <= drawCount_) {
            throw Lamb::Error::Code<RenderContext>("drawCount is over " + std::to_string(bufferSize), ErrorPlace);
        }

        drawData_[drawCount_].wvpMatrix.worldMat = vertexIndexData_->node.loacalMatrix * matrix.worldMat;
        drawData_[drawCount_].wvpMatrix.cameraMat = matrix.cameraMat;
    }
    inline void SetColor(const Vector4& color) override {
        if (bufferSize <= drawCount_) {
            throw Lamb::Error::Code<RenderContext>("drawCount is over " + std::to_string(bufferSize), ErrorPlace);
        }

        drawData_[drawCount_].color = color;
    }
    inline void SetLight(const DirectionLight& light) override {
        shaderData_[Lamb::GetGraphicBufferIndex()].light.MemCpy(&light);
    }
    inline void SetCameraPos(const Vector3& cameraPos) {
        shaderData_[Lamb::GetGraphicBufferIndex()].eyePos.MemCpy(cameraPos.data());
    }
    inline void SetShaderStruct(const T& data) {
        if (bufferSize <= drawCount_) {
            throw Lamb::Error::Code<RenderContext>("drawCount is over " + std::to_string(bufferSize), ErrorPlace);
        }
        drawData_[drawCount_].shaderStruct = data;
    }

    inline void ZSort() override {
        // 1以下ならソートする必要ないので早期リターン
        if (drawCount_ <= 1) {
            return;
        }


        // 正規化デバイス座標系にしてその深度値を入れる
        for (uint32_t i = 0; i < drawCount_; i++) {
            Mat4x4&& ndcMatrix = drawData_[i].wvpMatrix.worldMat * drawData_[i].wvpMatrix.cameraMat;
            // 深度値
            drawData_[i].depth = ndcMatrix.back().at(3);
        }
        // 描画をする部分だけソート
        auto endItr = drawData_.begin() + drawCount_;
        // 深度値でソート(大きい順。奥から描画していくため)
        std::sort(drawData_.begin(), endItr, [](const auto& left, const auto& right) {
            return right.depth < left.depth;
            }
        );

    }

    inline void SetData() override {
        shaderData_[Lamb::GetGraphicBufferIndex()].wvpMatrix.Map();
        shaderData_[Lamb::GetGraphicBufferIndex()].color.Map();
        shaderData_[Lamb::GetGraphicBufferIndex()].shaderStruct.Map();

        for (uint32_t i = 0; i < drawCount_; ++i) {
            shaderData_[Lamb::GetGraphicBufferIndex()].wvpMatrix[i]    = drawData_[i].wvpMatrix;
            shaderData_[Lamb::GetGraphicBufferIndex()].color[i]        = drawData_[i].color;
            shaderData_[Lamb::GetGraphicBufferIndex()].shaderStruct[i] = drawData_[i].shaderStruct;
        }

        shaderData_[Lamb::GetGraphicBufferIndex()].wvpMatrix.Unmap();
        shaderData_[Lamb::GetGraphicBufferIndex()].color.Unmap();
        shaderData_[Lamb::GetGraphicBufferIndex()].shaderStruct.Unmap();
    }


private:
    std::array<ShaderData<T>, DirectXSwapChain::kBackBufferNumber> shaderData_;
    std::vector<DrawData<T>> drawData_;
};

template<class T = uint32_t, uint32_t bufferSize = RenderData::kMaxDrawInstance>
class SkinRenderContext : public BaseRenderContext {
public:
    SkinRenderContext() :
        BaseRenderContext(),
        shaderData_()
    {
        shaderData_[Lamb::GetGraphicBufferIndex()].wvpMatrix.CreateBuffer(bufferSize);
        shaderData_[Lamb::GetGraphicBufferIndex()].color.CreateBuffer(bufferSize);
        shaderData_[Lamb::GetGraphicBufferIndex()].shaderStruct.CreateBuffer(bufferSize);

        drawData_.resize(bufferSize);
       

        pipeline_ = nullptr;
        drawCount_ = 0u;

        typeID_ = (typeid(SkinRenderContext<T, bufferSize>).name());
    }
    ~SkinRenderContext() = default;

    SkinRenderContext(const SkinRenderContext&) = delete;
    SkinRenderContext(SkinRenderContext&&) = delete;

    SkinRenderContext& operator=(const SkinRenderContext&) = delete;
    SkinRenderContext& operator=(SkinRenderContext&&) = delete;

public:
    void Draw() const override {
        // ディスクリプタヒープ
        CbvSrvUavHeap* const descriptorHeap = CbvSrvUavHeap::GetInstance();
        // コマンドリスト
        ID3D12GraphicsCommandList* const commandlist = DirectXCommand::GetMainCommandlist()->GetCommandList();

        // パイプライン設定
        pipeline_->Use();

        // ライト構造体
        commandlist->SetGraphicsRootConstantBufferView(0, shaderData_[Lamb::GetGraphicBufferIndex()].light.GetGPUVtlAdrs());
        // カメラポジション
        commandlist->SetGraphicsRootConstantBufferView(1, shaderData_[Lamb::GetGraphicBufferIndex()].eyePos.GetGPUVtlAdrs());
        // ワールドとカメラマトリックス
        commandlist->SetGraphicsRootShaderResourceView(2, shaderData_[Lamb::GetGraphicBufferIndex()].wvpMatrix.GetGPUVtlAdrs());
        // 色
        commandlist->SetGraphicsRootShaderResourceView(3, shaderData_[Lamb::GetGraphicBufferIndex()].color.GetGPUVtlAdrs());
        // shaderの固有構造体
        commandlist->SetGraphicsRootShaderResourceView(4, shaderData_[Lamb::GetGraphicBufferIndex()].shaderStruct.GetGPUVtlAdrs());
        // スキンアニメーション用
        commandlist->SetGraphicsRootShaderResourceView(5, skinCluster_->paletteBuffer[Lamb::GetGraphicBufferIndex()].GetGPUVtlAdrs());
        // テクスチャ
        commandlist->SetGraphicsRootDescriptorTable(6, descriptorHeap->GetGpuHeapHandle(0));

        std::array vertexBuffer = {
            vertexIndexData_->vertexView,
            skinCluster_->infliuenceBufferView
        };

        // 頂点バッファセット
        commandlist->IASetVertexBuffers(0, static_cast<uint32_t>(vertexBuffer.size()), vertexBuffer.data());
        // インデックスバッファセット
        commandlist->IASetIndexBuffer(&vertexIndexData_->indexView);
        // ドローコール
        commandlist->DrawIndexedInstanced(vertexIndexData_->indexNumber, drawCount_, 0, 0, 0);
    }

    void DrawShadow() const override {
        // コマンドリスト
        ID3D12GraphicsCommandList* const commandlist = DirectXCommand::GetMainCommandlist()->GetCommandList();

        // パイプライン設定
        shadowPipeline_->Use();

        // ライトのカメラマトリックス
        commandlist->SetGraphicsRootConstantBufferView(0, lightCamera_[Lamb::GetGraphicBufferIndex()]->GetGPUVtlAdrs());
        // ワールドとカメラマトリックス
        commandlist->SetGraphicsRootShaderResourceView(1, shaderData_[Lamb::GetGraphicBufferIndex()].wvpMatrix.GetGPUVtlAdrs());
        // スキンアニメーション用
        commandlist->SetGraphicsRootShaderResourceView(2, skinCluster_->paletteBuffer[Lamb::GetGraphicBufferIndex()].GetGPUVtlAdrs());

        std::array vertexBuffer = {
            vertexIndexData_->vertexView,
            skinCluster_->infliuenceBufferView
        };

        // 頂点バッファセット
        commandlist->IASetVertexBuffers(0, static_cast<uint32_t>(vertexBuffer.size()), vertexBuffer.data());
        // インデックスバッファセット
        commandlist->IASetIndexBuffer(&vertexIndexData_->indexView);
        // ドローコール
        commandlist->DrawIndexedInstanced(vertexIndexData_->indexNumber, drawCount_, 0, 0, 0);
    }

public:
    inline void SetVertexIndexData(const VertexIndexData* const vertexIndexData) override {
        if (!vertexIndexData) {
            throw Lamb::Error::Code<SkinRenderContext>("vertexIndexData is nullptr", ErrorPlace);
        }
        vertexIndexData_ = vertexIndexData;
    }
    inline void SetModelData(const ModelData* const modelData) override {
        if (!modelData) {
            throw Lamb::Error::Code<SkinRenderContext>("modelData is nullptr", ErrorPlace);
        }
        modelData_ = modelData;
    }
    inline void SetPipeline(Pipeline* const pipeline) override {
        if (!pipeline) {
            throw Lamb::Error::Code<SkinRenderContext>("pipeline is nullptr", ErrorPlace);
        }
        pipeline_ = pipeline;
    }
    inline void SetSkinCluster(SkinCluster* skinCluster) {
        if (not skinCluster) {
            throw Lamb::Error::Code<SkinRenderContext>("skinCluster is nullptr", ErrorPlace);
        }
        skinCluster_ = skinCluster;
    }
    inline void SetWVPMatrix(const WVPMatrix& matrix) override {
        if (bufferSize <= drawCount_) {
            throw Lamb::Error::Code<SkinRenderContext>("drawCount is over " + std::to_string(bufferSize), ErrorPlace);
        }

        drawData_[drawCount_].wvpMatrix.worldMat = matrix.worldMat;
        drawData_[drawCount_].wvpMatrix.cameraMat = matrix.cameraMat;
    }
    inline void SetColor(const Vector4& color) override {
        if (bufferSize <= drawCount_) {
            throw Lamb::Error::Code<SkinRenderContext>("drawCount is over " + std::to_string(bufferSize), ErrorPlace);
        }

        drawData_[drawCount_].color = color;
    }
    inline void SetLight(const DirectionLight& light) override {
        shaderData_[Lamb::GetGraphicBufferIndex()].light.MemCpy(&light);
    }
    inline void SetCameraPos(const Vector3& cameraPos) {
        shaderData_[Lamb::GetGraphicBufferIndex()].eyePos.MemCpy(cameraPos.data());
    }
    inline void SetShaderStruct(const T& data) {
        if (bufferSize <= drawCount_) {
            throw Lamb::Error::Code<SkinRenderContext>("drawCount is over " + std::to_string(bufferSize), ErrorPlace);
        }
        drawData_[drawCount_].shaderStruct = data;
    }

    inline void ZSort() override {
        // 1以下ならソートする必要ないので早期リターン
        if (drawCount_ <= 1) {
            return;
        }


        // 正規化デバイス座標系にしてその深度値を入れる
        for (uint32_t i = 0; i < drawCount_; i++) {
            Mat4x4&& ndcMatrix = drawData_[i].wvpMatrix.worldMat * drawData_[i].wvpMatrix.cameraMat;
            // 深度値
            drawData_[i].depth = ndcMatrix.back().at(3);
        }
        // 描画をする部分だけソート
        auto endItr = drawData_.begin() + drawCount_;
        // 深度値でソート(大きい順。奥から描画していくため)
        std::sort(drawData_.begin(), endItr, [](const auto& left, const auto& right) {
            return right.depth < left.depth;
            }
        );
    }

    inline void SetData() override {
        shaderData_[Lamb::GetGraphicBufferIndex()].wvpMatrix.Map();
        shaderData_[Lamb::GetGraphicBufferIndex()].color.Map();
        shaderData_[Lamb::GetGraphicBufferIndex()].shaderStruct.Map();

        for (uint32_t i = 0; i < drawCount_; ++i) {
            shaderData_[Lamb::GetGraphicBufferIndex()].wvpMatrix[i] = drawData_[i].wvpMatrix;
            shaderData_[Lamb::GetGraphicBufferIndex()].color[i] = drawData_[i].color;
            shaderData_[Lamb::GetGraphicBufferIndex()].shaderStruct[i] = drawData_[i].shaderStruct;
        }

        shaderData_[Lamb::GetGraphicBufferIndex()].wvpMatrix.Unmap();
        shaderData_[Lamb::GetGraphicBufferIndex()].color.Unmap();
        shaderData_[Lamb::GetGraphicBufferIndex()].shaderStruct.Unmap();
    }


private:
    std::array<ShaderData<T>, DirectXSwapChain::kBackBufferNumber> shaderData_;
    std::vector<DrawData<T>> drawData_;

    Lamb::SafePtr<SkinCluster> skinCluster_;
};

// MeshShaderテスト用
template<class T = uint32_t, uint32_t bufferSize = RenderData::kMaxDrawInstance>
class MeshRenderContext : public BaseRenderContext {
public:
    MeshRenderContext() :
        BaseRenderContext(),
        shaderData_()
    {
        constexpr uint32_t kBufferSize = bufferSize;
        drawData_.resize(kBufferSize);
       
        std::for_each(
            shaderStruct_.begin(),
            shaderStruct_.end(),
            [kBufferSize](auto& n) {
                n.CreateBuffer(kBufferSize);
            }
        );
        std::for_each(
            colors_.begin(),
            colors_.end(),
            [kBufferSize](auto& n) {
                n.CreateBuffer(kBufferSize);
            }
        );

        std::for_each(
            gTransform_.begin(),
            gTransform_.end(),
            [kBufferSize](auto& n) {
                n.CreateBuffer(kBufferSize);
            }
        );

        typeID_ = (typeid(MeshRenderContext).name());
    }
    ~MeshRenderContext() = default;

    MeshRenderContext(const MeshRenderContext&) = delete;
    MeshRenderContext(MeshRenderContext&&) = delete;

    MeshRenderContext& operator=(const MeshRenderContext&) = delete;
    MeshRenderContext& operator=(MeshRenderContext&&) = delete;

public:
    void Draw() const override {
        // ディスクリプタヒープ
        CbvSrvUavHeap* const descriptorHeap = CbvSrvUavHeap::GetInstance();
        
        // コマンドリスト
        Lamb::SafePtr commandlist = DirectXCommand::GetMainCommandlist()->GetCommandList();

        // パイプライン設定
        pipeline_->Use();
        // Light
        commandlist->SetGraphicsRootConstantBufferView(0, light_[Lamb::GetGraphicBufferIndex()].GetGPUVtlAdrs());
        // cameraPos
        commandlist->SetGraphicsRootConstantBufferView(1, eyePos_[Lamb::GetGraphicBufferIndex()].GetGPUVtlAdrs());
        // インスタンスカウント
        commandlist->SetGraphicsRootConstantBufferView(2, instanceCount_[Lamb::GetGraphicBufferIndex()].GetGPUVtlAdrs());


        // Transform
        commandlist->SetGraphicsRootShaderResourceView(3, gTransform_[Lamb::GetGraphicBufferIndex()].GetGPUVtlAdrs());
        // gVertices
        commandlist->SetGraphicsRootShaderResourceView(4, shaderData_->gVertices.GetGPUVtlAdrs());
        // gUniqueVertexIndices
        commandlist->SetGraphicsRootShaderResourceView(5, shaderData_->gUniqueVertexIndices.GetGPUVtlAdrs());
        // gPrimitiveIndices
        commandlist->SetGraphicsRootShaderResourceView(6, shaderData_->gPrimitiveIndices.GetGPUVtlAdrs());
        // gMeshlets
        commandlist->SetGraphicsRootShaderResourceView(7, shaderData_->gMeshlets.GetGPUVtlAdrs());
        // 色
        commandlist->SetGraphicsRootShaderResourceView(8, colors_[Lamb::GetGraphicBufferIndex()].GetGPUVtlAdrs());
        // 各shader固有の構造体
        commandlist->SetGraphicsRootShaderResourceView(9, shaderStruct_[Lamb::GetGraphicBufferIndex()].GetGPUVtlAdrs());
        // Textures
        commandlist->SetGraphicsRootDescriptorTable(10, descriptorHeap->GetGpuHeapHandle(0));
        // ドローコール
        commandlist->DispatchMesh(shaderData_->meshletCount, 1, 1);
    }

    void DrawShadow() const override {
        // コマンドリスト
        Lamb::SafePtr commandlist = DirectXCommand::GetMainCommandlist()->GetCommandList();

        // パイプライン設定
        shadowPipeline_->Use();
        // インスタンスカウント
        commandlist->SetGraphicsRootConstantBufferView(0, instanceCount_[Lamb::GetGraphicBufferIndex()].GetGPUVtlAdrs());
        // ライトマトリックス
        commandlist->SetGraphicsRootConstantBufferView(1, lightCamera_[Lamb::GetGraphicBufferIndex()]->GetGPUVtlAdrs());


        // Transform
        commandlist->SetGraphicsRootShaderResourceView(2, gTransform_[Lamb::GetGraphicBufferIndex()].GetGPUVtlAdrs());
        // gVertices
        commandlist->SetGraphicsRootShaderResourceView(3, shaderData_->gVertices.GetGPUVtlAdrs());
        // gUniqueVertexIndices
        commandlist->SetGraphicsRootShaderResourceView(4, shaderData_->gUniqueVertexIndices.GetGPUVtlAdrs());
        // gPrimitiveIndices
        commandlist->SetGraphicsRootShaderResourceView(5, shaderData_->gPrimitiveIndices.GetGPUVtlAdrs());
        // gMeshlets
        commandlist->SetGraphicsRootShaderResourceView(6, shaderData_->gMeshlets.GetGPUVtlAdrs());
       

        // ドローコール
        commandlist->DispatchMesh(shaderData_->meshletCount, 1, 1);
    }

public:
    inline void SetVertexIndexData(const VertexIndexData* const vertexIndexData) override {
        if (!vertexIndexData) {
            throw Lamb::Error::Code<MeshRenderContext>("vertexIndexData is nullptr", ErrorPlace);
        }
        vertexIndexData_ = vertexIndexData;
    }
    inline void SetModelData([[maybe_unused]]const ModelData* const modelData) override {
        
    }
    inline void SetPipeline(Pipeline* const pipeline) override {
        if (!pipeline) {
            throw Lamb::Error::Code<MeshRenderContext>("pipeline is nullptr", ErrorPlace);
        }
        pipeline_ = pipeline;
    }
    inline void SetWVPMatrix(const WVPMatrix& matrix) override {
        if (bufferSize <= drawCount_) {
            throw Lamb::Error::Code<MeshRenderContext>("drawCount is over " + std::to_string(bufferSize), ErrorPlace);
        }

        drawData_[drawCount_].wvpMatrix.worldMat = vertexIndexData_->node.loacalMatrix * matrix.worldMat;
        drawData_[drawCount_].wvpMatrix.cameraMat = matrix.cameraMat;
    }
    inline void SetColor(const Vector4& color) override {
        if (bufferSize <= drawCount_) {
            throw Lamb::Error::Code<MeshRenderContext>("drawCount is over " + std::to_string(bufferSize), ErrorPlace);
        }

        drawData_[drawCount_].color = color;
    }
    inline void SetLight(const DirectionLight& light) override {
        light_[Lamb::GetGraphicBufferIndex()].MemCpy(&light);
    }
    inline void SetCameraPos(const Vector3& cameraPos) {
        eyePos_[Lamb::GetGraphicBufferIndex()].MemCpy(cameraPos.data());
    }
    inline void SetShaderStruct(const T& data) {
        if (bufferSize <= drawCount_) {
            throw Lamb::Error::Code<MeshRenderContext>("drawCount is over " + std::to_string(bufferSize), ErrorPlace);
        }
        drawData_[drawCount_].shaderStruct = data;
    }

    inline void ZSort() override {
        // 1以下ならソートする必要ないので早期リターン
        if (drawCount_ <= 1) {
            return;
        }


        // 正規化デバイス座標系にしてその深度値を入れる
        for (uint32_t i = 0; i < drawCount_; i++) {
            Mat4x4&& ndcMatrix = drawData_[i].wvpMatrix.worldMat * drawData_[i].wvpMatrix.cameraMat;
            // 深度値
            drawData_[i].depth = ndcMatrix.back().at(3);
        }
        // 描画をする部分だけソート
        auto endItr = drawData_.begin() + drawCount_;
        // 深度値でソート(大きい順。奥から描画していくため)
        std::sort(drawData_.begin(), endItr, [](const auto& left, const auto& right) {
            return right.depth < left.depth;
            }
        );
    }

    inline void SetData() override {
        gTransform_[Lamb::GetGraphicBufferIndex()].Map();
        shaderStruct_[Lamb::GetGraphicBufferIndex()].Map();
        colors_[Lamb::GetGraphicBufferIndex()].Map();

        for (uint32_t i = 0; i < drawCount_; ++i) {
            gTransform_[Lamb::GetGraphicBufferIndex()][i] = drawData_[i].wvpMatrix;
            colors_[Lamb::GetGraphicBufferIndex()][i] = drawData_[i].color;
            shaderStruct_[Lamb::GetGraphicBufferIndex()][i] = drawData_[i].shaderStruct;
        }

        gTransform_[Lamb::GetGraphicBufferIndex()].Unmap();
        shaderStruct_[Lamb::GetGraphicBufferIndex()].Unmap();
        colors_[Lamb::GetGraphicBufferIndex()].Unmap();

        instanceCount_[Lamb::GetGraphicBufferIndex()].MemCpy(&drawCount_);
    }

    inline void SetMeshShaderData(Lamb::SafePtr<MeshShaderData> shaderData) {
        shaderData_ = shaderData;
    }

private:
    Lamb::SafePtr<MeshShaderData> shaderData_;

    std::array<ConstantBuffer<DirectionLight>, DirectXSwapChain::kBackBufferNumber> light_;
    std::array<ConstantBuffer<Vector3>, DirectXSwapChain::kBackBufferNumber> eyePos_;
    std::array<ConstantBuffer<uint32_t>, DirectXSwapChain::kBackBufferNumber> instanceCount_;
    std::array<StructuredBuffer<T>, DirectXSwapChain::kBackBufferNumber> shaderStruct_;
    std::array<StructuredBuffer<Vector4>, DirectXSwapChain::kBackBufferNumber> colors_;

    std::array<StructuredBuffer<WVPMatrix>, DirectXSwapChain::kBackBufferNumber> gTransform_;// トランスフォーム

    std::vector<DrawData<T>> drawData_;
};


template<class T>
concept IsBasedRenderContext =  std::is_base_of_v<BaseRenderContext, T>;

class RenderSet {
private:
    static constexpr int32_t kRenderDataIndexNum = (BlendType::kNum + 1);

public:
    RenderSet() = default;
    ~RenderSet() = default;
    RenderSet(const RenderSet&) = delete;
    RenderSet(RenderSet&&) = delete;

    RenderSet& operator=(const RenderSet&) = delete;
    RenderSet& operator=(RenderSet&&) = delete;


public:
    inline void Set(RenderData* const renderData, BlendType blend) {
        if (not renderData) {
            throw Lamb::Error::Code<RenderSet>("renderData is nullptr", ErrorPlace);
        }

        if (blend == BlendType::kAlphaEffect) {
            blend = BlendType::kNum;
        }

        renderDatas_[blend].reset(renderData);
    }

    RenderData* const GetRenderData(BlendType blend) const {
        if (blend == BlendType::kAlphaEffect) {
            blend = BlendType::kNum;
        }
        return renderDatas_[blend].get();
    }
    bool IsDraw(BlendType blend) const {
        if (blend == BlendType::kAlphaEffect) {
            blend = BlendType::kNum;
        }
        return renderDatas_[blend]->IsDraw();
    }

    inline void Draw() {
        for (auto& i : renderDatas_) {
            if (i->IsDraw()) {
                i->Draw();
            }
        }
    }

    inline void ResetDrawCount() {
        for (auto& i : renderDatas_) {
            i->ResetDrawCount();
        }
    }

    inline void DrawAndResetDrawCount() {
        Draw();
        ResetDrawCount();
    }

    inline void SetData() {
        for (auto& i : renderDatas_) {
            i->SetData();
        }
    }

public:
    inline RenderData* GetRenderContext(BlendType blend) {
        if (blend == BlendType::kAlphaEffect) {
            blend = BlendType::kNum;
        }
        return renderDatas_[blend].get();
    }

    // ごり押し
    // やばい
    // マジでヤバイ
    // 使うときは気を付けて(一応型が違ったらエラーは出る)
    template<IsBasedRenderContext ClassName>
    inline ClassName* GetRenderContextDowncast(BlendType blend) {
        if (blend == BlendType::kAlphaEffect) {
            blend = BlendType::kNum;
        }
        if (typeid(ClassName).name() != renderDatas_[blend]->GetID()) {
            throw Lamb::Error::Code<RenderSet>("does not match class type", ErrorPlace);
        }

        return dynamic_cast<ClassName*>(renderDatas_[blend].get());
    }

    const std::string& GetRootNodeName() const {
        return renderDatas_.front()->GetRootName();
    }

    const Node& GetNode() const {
        return renderDatas_.front()->GetNode();
    }

    const VertexIndexData* const GetMesh() const {
        return renderDatas_.front()->GetVertexIndexData();
    }
    const ModelData* const GetModelData() const {
        return renderDatas_.front()->GetModelData();
    }

public:
    std::array<std::unique_ptr<RenderData>, kRenderDataIndexNum>::iterator begin() {
        return renderDatas_.begin();
    }
    std::array<std::unique_ptr<RenderData>, kRenderDataIndexNum>::iterator end() {
        return renderDatas_.end();
    }
    std::array<std::unique_ptr<RenderData>, kRenderDataIndexNum>::const_iterator cbegin() const {
        return renderDatas_.cbegin();
    }
    std::array<std::unique_ptr<RenderData>, kRenderDataIndexNum>::const_iterator cend() const {
        return renderDatas_.cend();
    }
    std::array<std::unique_ptr<RenderData>, kRenderDataIndexNum>::reverse_iterator rbegin() {
        return renderDatas_.rbegin();
    }
    std::array<std::unique_ptr<RenderData>, kRenderDataIndexNum>::reverse_iterator rend() {
        return renderDatas_.rend();
    }
    std::array<std::unique_ptr<RenderData>, kRenderDataIndexNum>::const_reverse_iterator crbegin() const {
        return renderDatas_.crbegin();
    }
    std::array<std::unique_ptr<RenderData>, kRenderDataIndexNum>::const_reverse_iterator crend() const {
        return renderDatas_.crend();
    }

    std::unique_ptr<RenderData>* const data() {
        return renderDatas_.data();
    }
    const std::unique_ptr<RenderData>* const data() const {
        return renderDatas_.data();
    }

private:
    std::array<std::unique_ptr<RenderData>, kRenderDataIndexNum> renderDatas_;
};