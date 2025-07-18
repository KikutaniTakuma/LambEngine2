#include "Post.hlsli"

struct OutlineData{
    float32_t weight;
    float32_t4x4 projectionInverse;
};

ConstantBuffer<OutlineData> kOutlineData : register(b1);

Texture2D<float32_t> depthTexture : register(t1);
SamplerState pSmp : register(s1);

static const float32_t kPrewittHorizontalKernel[3][3] = {
    {-1.0f / 6.0f, 0.0f, 1.0f / 6.0f},
    {-1.0f / 6.0f, 0.0f, 1.0f / 6.0f},
    {-1.0f / 6.0f, 0.0f, 1.0f / 6.0f},
};

static const float32_t kPrewittVerticalKernel[3][3] = {
    {-1.0f / 6.0f, -1.0f / 6.0f, -1.0f / 6.0f },
    { 0.0f, 0.0f, 0.0f },
    { 1.0f / 6.0f, 1.0f / 6.0f, 1.0f / 6.0f},
};

static const float32_t2 kIndex[3][3] = {
    { { -1.0f, -1.0f }, { 0.0f, -1.0f }, { 1.0f, -1.0f } },
    { { -1.0f,  0.0f }, { 0.0f,  0.0f }, { 1.0f,  0.0f } },
    { { -1.0f,  1.0f }, { 0.0f,  1.0f }, { 1.0f,  1.0f } },
};

float32_t4 main(Output input) : SV_TARGET0 {
    float32_t2 difference = float32_t2(0.0f, 0.0f);
    uint32_t width, height;
    depthTexture.GetDimensions(width, height);
    float32_t2 uvStepSize = float32_t2(rcp(width), rcp(height));

    float32_t d = depthTexture.Sample(pSmp, input.uv);
    if(d < 1.0e-8){
        discard;
    }

    for(int32_t x = 0; x < 3; ++x){
        for(int32_t y = 0; y < 3; ++y){
            float32_t2 texcoord = input.uv + kIndex[x][y] * uvStepSize;
            float32_t depth = depthTexture.Sample(pSmp, texcoord);
            float32_t4 viewSpace = mul(float32_t4(0.0f, 0.0f, depth, 1.0f), kOutlineData.projectionInverse);
            float32_t viewZ = viewSpace.z * rcp(viewSpace.w);
            difference.x += viewZ * kPrewittHorizontalKernel[x][y];
            difference.y += viewZ * kPrewittVerticalKernel[x][y];
        }
    }

    float32_t weight = length(difference);
    weight = saturate(weight * kOutlineData.weight);

    float32_t4 output;
    output.rgb = min(1.0f - weight, 0.05f);
    output.a = weight;

    return output;
}