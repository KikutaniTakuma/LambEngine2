#include "Model.hlsli"

struct Well{
	float32_t4x4 skeletonSpaceMatrix;
	float32_t4x4 skeletonSpaceInverseTransposeMatrix;
};

StructuredBuffer<Well> kMatrixPalette : register(t4);

struct AnimationVertexInput{
	float32_t4 position : POSITION0;
	float32_t3 normal : NORMAL0;
	float32_t2 uv : TEXCOORD;
	uint32_t textureID : BLENDINDICES;
	float32_t4 weight0 : WEIGHT0;
	float32_t4 weight1 : WEIGHT1;
	int32_t4 index0 : INDEX0;
	int32_t4 index1 : INDEX1;
};

struct Skinned {
	float32_t4 position;
	float32_t3 normal;
};

Skinned Skinning(AnimationVertexInput input){
	Skinned result;
	result.position  = mul(input.position, kMatrixPalette[input.index0.x].skeletonSpaceMatrix) * input.weight0.x;
	result.position += mul(input.position, kMatrixPalette[input.index0.y].skeletonSpaceMatrix) * input.weight0.y;
	result.position += mul(input.position, kMatrixPalette[input.index0.z].skeletonSpaceMatrix) * input.weight0.z;
	result.position += mul(input.position, kMatrixPalette[input.index0.w].skeletonSpaceMatrix) * input.weight0.w;
	result.position += mul(input.position, kMatrixPalette[input.index1.x].skeletonSpaceMatrix) * input.weight1.x;
	result.position += mul(input.position, kMatrixPalette[input.index1.y].skeletonSpaceMatrix) * input.weight1.y;
	result.position += mul(input.position, kMatrixPalette[input.index1.z].skeletonSpaceMatrix) * input.weight1.z;
	result.position += mul(input.position, kMatrixPalette[input.index1.w].skeletonSpaceMatrix) * input.weight1.w;
	result.position.w = 1.0f;

	result.normal  = mul(input.normal, (float32_t3x3)kMatrixPalette[input.index0.x].skeletonSpaceInverseTransposeMatrix) * input.weight0.x;
	result.normal += mul(input.normal, (float32_t3x3)kMatrixPalette[input.index0.y].skeletonSpaceInverseTransposeMatrix) * input.weight0.y;
	result.normal += mul(input.normal, (float32_t3x3)kMatrixPalette[input.index0.z].skeletonSpaceInverseTransposeMatrix) * input.weight0.z;
	result.normal += mul(input.normal, (float32_t3x3)kMatrixPalette[input.index0.w].skeletonSpaceInverseTransposeMatrix) * input.weight0.w;
	result.normal += mul(input.normal, (float32_t3x3)kMatrixPalette[input.index1.x].skeletonSpaceInverseTransposeMatrix) * input.weight1.x;
	result.normal += mul(input.normal, (float32_t3x3)kMatrixPalette[input.index1.y].skeletonSpaceInverseTransposeMatrix) * input.weight1.y;
	result.normal += mul(input.normal, (float32_t3x3)kMatrixPalette[input.index1.z].skeletonSpaceInverseTransposeMatrix) * input.weight1.z;
	result.normal += mul(input.normal, (float32_t3x3)kMatrixPalette[input.index1.w].skeletonSpaceInverseTransposeMatrix) * input.weight1.w;
	result.normal = normalize(result.normal);

	return result;
}


VertexShaderOutput main(AnimationVertexInput input,uint32_t instanceID : SV_InstanceID)
{
	VertexShaderOutput output;
	Skinned skinned = Skinning(input);

	skinned.position = mul(skinned.position, kWvpMat[instanceID].worldMat);
	output.worldPosition = skinned.position;
	output.position = mul(skinned.position, kWvpMat[instanceID].cameraMat);
	output.normal = mul(skinned.normal, (float32_t3x3)kWvpMat[instanceID].worldMat);
	output.uv = input.uv;
	output.instanceID = instanceID;
	output.textureID = input.textureID;

	return output;
}