#include "Post.hlsli"
#include "../OutputStructs.hlsli"
#include "../Tonemap.hlsli"
#include "../PerlinNoise.hlsli"

Texture2D<float32_t4> gDistortionTexture : register(t1);
Texture2D<float32_t> gDepthTexture : register(t2);
Texture2D<float32_t4> gCausticsTexture : register(t3);
Texture2D<float32_t4> gWorldPositionTexture : register(t4);

struct Mat4x4{
    float32_t4x4 value;
};

struct Float{
    float32_t value;
};

ConstantBuffer<Mat4x4> gUVMatrix : register(b1);
ConstantBuffer<Float> gDepthFloat : register(b2);
ConstantBuffer<TonemapParams> gTonemapParams : register(b3);
ConstantBuffer<Mat4x4> gWaterMatrixInverse : register(b4);

// ポイントサンプラー
SamplerState gPointSmp : register(s1);

PixelShaderOutPut2 main(Output input) {
    PixelShaderOutPut2 output;


    float32_t depth = gDepthTexture.Sample(gPointSmp, input.uv);
    depth = 1.0f - pow(depth, 120.0f);

    float32_t2 uvDistortion = gDistortionTexture.Sample(gPointSmp, input.uv).xy;
    float32_t2 scrollDistortion = gDistortionTexture.Sample(gPointSmp, input.uv + uvDistortion).xy;
    if(scrollDistortion.x == 0.0f && scrollDistortion.y == 0.0f) {
        uvDistortion = 0;
    }

    float32_t2 uv = input.uv + uvDistortion;
    uv = saturate(uv);

    float32_t4 color = tex.Sample(gPointSmp, uv);
    
    // コースティクス加算
    // コースティクステクスチャのUV計算
    float32_t4 worldPostion = gWorldPositionTexture.Sample(gPointSmp, uv);
    float32_t waterSurfaceLength = -gWaterMatrixInverse.value[3][1] - worldPostion.y;
    float32_t2 waterUV = 0;
    if(worldPostion.y < -gWaterMatrixInverse.value[3][1] && !(scrollDistortion.x == 0.0f && scrollDistortion.y == 0.0f)){
        worldPostion.y = 0.0f;
        worldPostion.w = 1.0f;
        float32_t4 waterLocalPostion = mul(worldPostion, gWaterMatrixInverse.value);
        waterUV = waterLocalPostion.xz;
        waterUV.y *= -1.0f;
        waterUV = (waterUV + 1.0f) * 0.5f;
        float32_t4 causticsUV = 0;
        causticsUV.xy = waterUV;
        causticsUV.z = 0.0f;
        causticsUV.w = 1.0f;
        causticsUV = mul(causticsUV, gUVMatrix.value);

        float32_t4 caustics = gCausticsTexture.Sample(smp, causticsUV.xy + uvDistortion);
        //color.rgb += caustics.rgb;
        
    }
    // 泡
    // 水面との距離が一定以下だったら
    //if(0.0f < waterSurfaceLength && waterSurfaceLength < gDepthFloat.value) {
        //float32_t3 bubble = Perlin(100.0f, waterUV.xy);
        //color.rgb += bubble;
    //}

    

    float32_t luminate = dot(color.rgb, float32_t3(0.2627f,0.678f,0.0593f));
    output.color0.rgb = Tonemap(gTonemapParams, float32_t3(luminate,luminate,luminate)) * rcp(float32_t3(luminate,luminate,luminate)) * color.rgb * kColor.color.rgb;
    output.color0.w = 1.0f;
    output.color1 = output.color0;

    return output;
}