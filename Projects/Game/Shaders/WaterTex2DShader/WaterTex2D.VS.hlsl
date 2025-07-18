#include "WaterTex2D.hlsli"

VertexShaderOutputToHull main(VertexShaderInput input,uint32_t instanceID : SV_InstanceID)
{
    VertexShaderOutputToHull output;

	output.worldPosition = input.position;
	output.position =output.worldPosition;
	output.normal = mul(input.normal, (float32_t3x3)kWvpMat[instanceID].worldMat);
	output.normal = normalize(output.normal);

	output.uv = input.uv;
	output.instanceID = instanceID;
	output.textureID = input.textureID;

    return output;
}