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
struct MSInput {
	float32_t4 position;
	float32_t3 normal;
	float32_t2 uv;
	uint32_t textureID;
	float32_t2 pad;
};
struct Meshlet {
	uint32_t vertexCount;
	uint32_t vertexOffset;
	uint32_t primitiveCount;
	uint32_t primitiveOffset;
};

struct InstanceCount{
	uint32_t value;
};

ConstantBuffer<DirectionLight> kLight : register(b0);
ConstantBuffer<CameraPos> kCameraPos : register(b1);
ConstantBuffer<InstanceCount> kInstanceCount : register(b2);
StructuredBuffer<WVPMatrix> gTransform : register(t0);
StructuredBuffer<MSInput> gVertices : register(t1);
StructuredBuffer<uint32_t> gUniqueVertexIndices : register(t2);
StructuredBuffer<uint32_t> gPrimitiveIndices : register(t3);
StructuredBuffer<Meshlet> gMeshlets : register(t4);
StructuredBuffer<Color> gColor : register(t5);
Texture2D<float32_t4> textures[] : register(t7);
SamplerState smp : register(s0);


struct MSOutput {
	float32_t4 position : SV_POSITION;
    float32_t3 normal : NORMAL;
    float32_t4 worldPosition : POSITION1;
    float32_t2 uv : TEXCOORD0;
	uint32_t textureID : BLENDINDICES0;
	uint32_t instanceID : BLENDINDICES1;
};

struct PayloadStruct { 
    uint32_t myArbitraryData; 
}; 

/// Please define the following in each Shader ///

/// Please define the structure used in each Shader
/// example : 
///	struct IsLighting {
///		uint32_t isLighting;
///	};

/// After defining the structure used in each shader, 
/// define StructuredBuffer and specify register(t6).
/// example : 
/// StructuredBuffer<IsLighting> kIsLighting : register(t6);