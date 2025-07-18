#include "../LambMesh.hlsli"

struct IsLighting {
	uint32_t isLighting;
};

StructuredBuffer<IsLighting> gIsLighting : register(t6);

PixelShaderOutPut4 main(MSOutput input)
{
	float32_t4 textureColor = textures[input.textureID < 128 ? input.textureID : 0].Sample(smp, input.uv);

	PixelShaderOutPut4 output;
	output.color0 = textureColor;

	output.color0 = textureColor;

	if(gIsLighting[input.instanceID].isLighting == 1) {
		float32_t3 ligDirection = kLight.ligDirection;
        float32_t3 ligColor = kLight.ligColor;
        float32_t shinness = kLight.shinness;
		float32_t3 eyePos = kCameraPos.pos;

		// ディレクションライト拡散反射光
        float32_t t = dot(input.normal, ligDirection);
        t = saturate(t);

        float32_t3 diffDirection = ligColor * t;

		// 鏡面反射光
        float32_t3 toEye = eyePos - input.worldPosition.xyz;
        toEye = normalize(toEye);
    
        float32_t3 refVec = reflect(ligDirection, input.normal);
        refVec = normalize(refVec);

        t = dot(refVec, toEye);

        t = pow(saturate(t), shinness);
        float32_t3 specDirection = ligColor * t;
    
        float32_t3 lig = diffDirection + specDirection;
        lig += 0.2f;
        output.color0.rgb *= lig;
	}
	output.color0 *= gColor[input.instanceID].color;

	output.color2.xyz = input.normal;
	output.color2.w = 1.0f;

	output.color3.xyz = input.worldPosition.xyz;
	output.color3.w = 1.0f;

	output.color1 = 0;

	return output;
}