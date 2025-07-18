#include "SkyBox.hlsli"

TextureCube<float32_t4> gCubeTex : register(t0);
SamplerState smp : register(s0);

PixelOutPut main(VertexOutput input)
{
	PixelOutPut output;

	float32_t4 textureColor = gCubeTex.Sample(smp, input.texcoord);

	output.color = textureColor * gMaterialData.color;

	return output;
}