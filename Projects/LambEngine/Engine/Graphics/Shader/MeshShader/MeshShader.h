/// =====================
/// == MeshShader関連  ==
/// =====================



#pragma once
#include <d3d12.h>
#pragma comment(lib, "d3d12.lib")
#undef max
#undef min
#include "Engine/Buffer/ConstantBuffer.h" 
#include "Engine/Buffer/StructuredBuffer.h"
#include "Math/Vector4.h"
#include "Math/Vector3.h"
#include "Math/Vector2.h"
#include "Math/Matrix.h"

#include "../../GraphicsStructs.h"

#include "DirectXMesh/DirectXMesh.h"

/// ==========================================================================================
/// 参考書籍 : Direct3D12 ゲームグラフィックス実践ガイド
/// https://www.amazon.co.jp/Direct3D12-%E3%82%B2%E3%83%BC%E3%83%A0%E3%82%B0%E3%83%A9%E3%83%95%E3%82%A3%E3%83%83%E3%82%AF%E3%82%B9%E5%AE%9F%E8%B7%B5%E3%82%AC%E3%82%A4%E3%83%89-Pocol/dp/4297123657/ref=sr_1_2?__mk_ja_JP=%E3%82%AB%E3%82%BF%E3%82%AB%E3%83%8A&crid=1PR7D4S7HUYK9&dib=eyJ2IjoiMSJ9.JXb5uWtfduLdTkkKZ8a8dle0ix16NnZyXXe4ESKNhAJPeG6fixs668jY_eNY_EV9wXBRLP35iZtjRobAGa6cIweVx4dunLzrHnbi-U7qb2GAPkuzuoShFLogyRQUnb4bsfDwGptKCixGYe1QtQW5B8IFiImXp3qRrNQ2E6hHe0Cd3S5KxLBRnKg_U9wIhdDjPSK9PwWMMBtaLrkr9sQLo-P09CC9fgqOoJpUGGI5hIhMJ1-8rVP6LslD1_r4RtKCy9qlzId3FhA1ULOL6Zqrt-sBwqmPRT1agEv9TgflTBE.cWX7kt42MNWRyMR0Yktnm5bGVfBmxSzHHjBkpbIFQTA&dib_tag=se&keywords=direct+3d&qid=1728131902&sprefix=direct3d%2Caps%2C197&sr=8-2
/// ==========================================================================================


// MeshShaderのパイプラインを作るためのクラス
template<typename ValueType, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE kObjectType>
class alignas(void*) StateParam {
public:
	StateParam() :
		type_(kObjectType),
		value_(ValueType()) 
	{}

	StateParam(const ValueType& value) :
		type_(kObjectType),
		value_(value)
	{}

	~StateParam() = default;

	StateParam& operator=(const ValueType& value) {
		type_ = kObjectType;
		value_ = value;

		return *this;
	}


private:
	D3D12_PIPELINE_STATE_SUBOBJECT_TYPE type_;
	ValueType                           value_;
};

// MeshShaderのパイプラインを作るためのクラス(パディングあり)
template<typename ValueType, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE kObjectType>
class alignas(void*) StateParamHasPad {
public:
	StateParamHasPad() :
		type_(kObjectType),
		value_(ValueType()),
		pad({})
	{}

	StateParamHasPad(const ValueType& value) :
		type_(kObjectType),
		value_(value),
		pad({})
	{}

	~StateParamHasPad() = default;

	StateParamHasPad& operator=(const ValueType& value) {
		type_ = kObjectType;
		value_ = value;

		return *this;
	}


private:
	D3D12_PIPELINE_STATE_SUBOBJECT_TYPE type_;
	ValueType                           value_;
	int8_t                              pad[4]; // 4バイトパディング
};




using SP_ROOT_SIGNATURE = StateParam<       ID3D12RootSignature*,        D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_ROOT_SIGNATURE       >;
using SP_AS             = StateParam<       D3D12_SHADER_BYTECODE,       D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_AS                   >;
using SP_MS             = StateParam<       D3D12_SHADER_BYTECODE,       D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_MS                   >;
using SP_PS             = StateParam<       D3D12_SHADER_BYTECODE,       D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_PS                   >;
using SP_BLEND          = StateParamHasPad< D3D12_BLEND_DESC,            D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_BLEND                >;
using SP_RASTERIZER     = StateParam<       D3D12_RASTERIZER_DESC,       D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_RASTERIZER           >;
using SP_DEPTH_STENCIL  = StateParam<       D3D12_DEPTH_STENCIL_DESC,    D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_DEPTH_STENCIL        >;
using SP_SAMPLE_MASK    = StateParam<       UINT,                        D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_SAMPLE_MASK          >;
using SP_SAMPLE_DESC    = StateParamHasPad< DXGI_SAMPLE_DESC,            D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_SAMPLE_DESC          >;
using SP_RT_FORMAT      = StateParam<       D3D12_RT_FORMAT_ARRAY,       D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_RENDER_TARGET_FORMATS>;
using SP_DS_FORMAT      = StateParam<       DXGI_FORMAT,                 D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_DEPTH_STENCIL_FORMAT >;
using SP_FLAGS          = StateParam<       D3D12_PIPELINE_STATE_FLAGS,  D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_FLAGS                >;


// MeshShader用のパイプラインステート
struct MeshShaderPipelineStateDesc {
	SP_ROOT_SIGNATURE rootSignature;
	SP_AS             AS;
	SP_MS             MS;
	SP_PS             PS;
	SP_BLEND          blend;
	SP_RASTERIZER     rasterizer;
	SP_DEPTH_STENCIL  depthStencil;
	SP_SAMPLE_MASK    sampleMask;
	SP_SAMPLE_DESC    sampleDesc;
	SP_RT_FORMAT      RTFormat;
	SP_DS_FORMAT      DSFormt;
	SP_FLAGS          flags;
};

// Meshletを保存した構造体
struct ResMesh {
	std::vector<Vertex>           vertices; // 頂点データ
	std::vector<uint32_t>         indices;  // インデックスデータ
	
	std::vector<DirectX::Meshlet> meshlets;            // メッシュレット
	std::vector<uint32_t>         uniqueVertexIndices; // インデックスデータ
	std::vector<uint32_t>         primitiveIndices;    // プリミティブインデックス
};

// MeshShaderで使うStructuredBufferをまとめた構造体
struct MeshShaderData {
	StructuredBuffer<Vertex>           gVertices;            // 頂点データ
	StructuredBuffer<uint32_t>         gUniqueVertexIndices; // インデックスデータ
	StructuredBuffer<uint32_t>         gPrimitiveIndices;    // プリミティブインデックス
	StructuredBuffer<DirectX::Meshlet> gMeshlets;            // メッシュレット

	uint32_t meshletCount = 0; // メッシュレットの数
};