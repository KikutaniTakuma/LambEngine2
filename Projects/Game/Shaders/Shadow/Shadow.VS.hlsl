// Vertex shader input
struct VertexShaderInput {
	float32_t4 position : POSITION0;
	float32_t3 normal : NORMAL0;
	float32_t2 uv : TEXCOORD;
	uint32_t textureID : BLENDINDICES;
	float32_t2 pad: NORMAL1;
};

struct LightCamera {
	float32_t4x4 mat;
};

struct WVPMatrix {
	float32_t4x4 worldMat;
	float32_t4x4 cameraMat;
};

ConstantBuffer<LightCamera> gCameraMat : register(b0);
StructuredBuffer<WVPMatrix> gWvpMat : register(t0);

struct ShadowOutput{
	float32_t4 pos : SV_POSITION;
};

ShadowOutput main(VertexShaderInput input,uint32_t instanceID : SV_InstanceID)
{
	ShadowOutput output;
	
	float32_t4 worldPos = mul(input.position, gWvpMat[instanceID].worldMat);
	output.pos = mul(worldPos, gCameraMat.mat);

	return output;
}