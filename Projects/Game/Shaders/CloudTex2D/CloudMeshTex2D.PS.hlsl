#include "../LambMesh.hlsli"
#include "../PerlinNoise.hlsli"

struct Texture2DData {
    float32_t4x4 uvTransform;
    float32_t3 vec;
    uint32_t textureID;
};

StructuredBuffer<Texture2DData> kTexture2DData : register(t6);

PixelShaderOutPut4 main(MSOutput input)
{
	PixelShaderOutPut4 output;

    float32_t perlinNoise = FractalSumNnoise(kTexture2DData[input.instanceID].vec.x, input.uv);
    
    uint32_t textureID = kTexture2DData[input.instanceID].textureID;
	
	float32_t4 textureColor = textures[textureID].Sample(smp, input.uv);
    if(textureColor.w < 0.1f 
        || perlinNoise < kTexture2DData[input.instanceID].vec.y
    ){
        discard;
    }

    output.color0 = textureColor * gColor[input.instanceID].color;
    output.color0.w *= min(perlinNoise, kTexture2DData[input.instanceID].vec.z);

    // 法線
    output.color2.xyz = input.normal;
    output.color2.w = 1.0f;

    // ポジション
    output.color3 = input.worldPosition;

    output.color1 = 0;
    
	return output;
}