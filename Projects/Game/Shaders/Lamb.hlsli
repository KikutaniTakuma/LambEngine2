#include "OutputStructs.hlsli"

struct WVPMatrix {
	float32_t4x4 worldMat;
	float32_t4x4 cameraMat;
};

struct DirectionLight {
    float32_t3 ligDirection;
    float32_t shinness;
    float32_t3 ligColor;
    float32_t pad0;
};

struct CameraPos{
	float32_t3 pos;
};

struct Color {
	float32_t4 color;
};

ConstantBuffer<DirectionLight> kLight : register(b0);
ConstantBuffer<CameraPos> kCameraPos : register(b1);
StructuredBuffer<WVPMatrix> kWvpMat : register(t0);
StructuredBuffer<Color> kColor : register(t1);
Texture2D<float32_t4> textures[] : register(t3);
SamplerState smp : register(s0);

// Vertex shader input
struct VertexShaderInput {
	float32_t4 position : POSITION0;
	float32_t3 normal : NORMAL0;
	float32_t2 uv : TEXCOORD;
	uint32_t textureID : BLENDINDICES;
	float32_t2 pad: NORMAL1;
};

// Vertex shader out put to Rasterizer
struct VertexShaderOutput{
    float32_t4 position : SV_POSITION;
    float32_t3 normal : NORMAL;
    float32_t4 worldPosition : POSITION1;
    float32_t2 uv : TEXCOORD0;
	uint32_t textureID : BLENDINDICES0;
	uint32_t instanceID : BLENDINDICES1;
};
// Vertex shader out put to Hull shader
struct VertexShaderOutputToHull{
    float32_t4 position : POSITION;
    float32_t3 normal : NORMAL;
    float32_t4 worldPosition : POSITION1;
    float32_t2 uv : TEXCOORD0;
	uint32_t textureID : BLENDINDICES0;
	uint32_t instanceID : BLENDINDICES1;
};

// Hull shader out put to Domain shader
struct HullShaderOutPut{
    float32_t4 position : SV_POSITION;
    float32_t3 normal : NORMAL;
	float32_t4 worldPosition : POSITION1;
    float32_t2 uv : TEXCOORD0;
	uint32_t textureID : BLENDINDICES0;
	uint32_t instanceID : BLENDINDICES1;
};
struct ConstantHullShaderOutPut
{
	float32_t Edges[3] : SV_TessFactor;
	float32_t Inside : SV_InsideTessFactor;
};

// Domain shader out put to Rasterizer
struct DomainShaderOutPut{
    float32_t4 position : SV_POSITION;
    float32_t3 normal : NORMAL;
	float32_t4 worldPosition : POSITION1;
    float32_t2 uv : TEXCOORD0;
	uint32_t textureID : BLENDINDICES0;
	uint32_t instanceID : BLENDINDICES1;
};
// Domain shader out put to Geometory shader
struct DomainShaderOutPutToGeometory{
    float32_t4 position : POSITION;
    float32_t3 normal : NORMAL;
	float32_t4 worldPosition : POSITION1;
    float32_t2 uv : TEXCOORD0;
	uint32_t textureID : BLENDINDICES0;
	uint32_t instanceID : BLENDINDICES1;
};

// Geometory shader out put to Rasterizer
struct GeometoryOutPut{
    float32_t4 position : SV_POSITION;
    float32_t3 normal : NORMAL;
	float32_t4 worldPosition : POSITION1;
    float32_t2 uv : TEXCOORD0;
	uint32_t textureID : BLENDINDICES0;
	uint32_t instanceID : BLENDINDICES1;
};


/// Please define the following in each Shader ///

/// Please define the structure used in each Shader
/// example : 
///	struct IsLighting {
///		uint32_t isLighting;
///	};

/// After defining the structure used in each shader, 
/// define StructuredBuffer and specify register(t2).
/// example : 
/// StructuredBuffer<IsLighting> kIsLighting : register(t2);