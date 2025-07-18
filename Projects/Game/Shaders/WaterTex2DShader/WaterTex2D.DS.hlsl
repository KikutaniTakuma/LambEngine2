#include "WaterTex2D.hlsli"

[domain("tri")]
DomainShaderOutPutToGeometory main(ConstantHullShaderOutPut input,  float32_t3 domain : SV_DomainLocation, const OutputPatch<HullShaderOutPut, 3> patch){
    DomainShaderOutPutToGeometory output;

    output.position = 
		patch[0].position * domain.x 
		+ patch[1].position * domain.y 
		+ patch[2].position * domain.z;
	output.worldPosition = output.position;
	
	output.normal = patch[0].normal;
	output.uv = 
		patch[0].uv * domain.x 
		+ patch[1].uv * domain.y 
		+ patch[2].uv * domain.z;

	output.textureID = patch[0].textureID;
	output.instanceID = patch[0].instanceID;

    return output;
}