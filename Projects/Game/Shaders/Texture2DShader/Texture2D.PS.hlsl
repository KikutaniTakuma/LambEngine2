#include "Texture2D.hlsli"

PixelShaderOutPut4 main(VertexShaderOutput input)
{
	PixelShaderOutPut4 output;

    uint32_t textureID = kTexture2DData[input.instanceID].textureID;
	
	float32_t4 textureColor = textures[textureID].Sample(smp, input.uv);
    if(textureColor.w < 0.1f){
        discard;
    }

	output.color0 = textureColor * kColor[input.instanceID].color;

    // 法線
    output.color2.xyz = input.normal;
    output.color2.w = 1.0f;

    // ポジション
    output.color3 = input.worldPosition;

    output.color1 = 0;
    
	return output;
}