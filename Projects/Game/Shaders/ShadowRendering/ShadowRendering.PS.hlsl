#include "ShadowRendering.hlsli"
#include "../OutputStructs.hlsli"

//Texture2D<float32_t4> gColorTexture : register(t0);
Texture2D<float32_t> gDepth : register(t0);
Texture2D<float32_t> gDepthShadow : register(t1);

// ポイントサンプラー
SamplerState gPointSmp : register(s0);

PixelShaderOutPut main(Output input) {
PixelShaderOutPut outputColor;

    float32_t shadow = gDepthShadow.Sample(gPointSmp, input.uv);

    outputColor.color.rgb = 1.0f - pow(shadow, 120.0f);
    outputColor.color.a = 1.0f;

    return outputColor;
}