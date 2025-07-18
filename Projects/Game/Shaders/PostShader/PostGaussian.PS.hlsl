#include "Post.hlsli"

static const float32_t PI = 3.14159265f;

struct GaussianBlurState{
    float32_t2 dir;
    float32_t sigma;
    int32_t kernelSize;
};

ConstantBuffer<GaussianBlurState> kGaussianBlurState : register(b1);

float32_t gauss(float32_t x, float32_t y, float32_t sigma){
    float32_t exponent = -(x * x + y* y) * rcp(2.0f * sigma * sigma);
    float32_t denominator = 2.0f * PI * sigma * sigma;
    return exp(exponent) * rcp(denominator);
}

float32_t gauss1Dimantion(float32_t x, float32_t sigma){
    return 1.0f / (sqrt(2.0f * PI) * sigma) * exp(-(x*x)  / (2.0f * sigma * sigma));
}

float32_t4 GaussianBlur(
    float32_t2 texelSize,
    float32_t2 uv, 
    int32_t kernelSize,
    float32_t sigma, 
    float32_t2 dir
    )
{
    float32_t4 output = float32_t4(0.0f,0.0f,0.0f,0.0f);
    float32_t sum = 0.0f;
    float32_t2 uvOffset;
    float32_t weight = 0.0f;

    for(int kernelStep = -kernelSize / 2; kernelStep <= kernelSize / 2;  kernelStep += 2) {
        uvOffset = uv;
        uvOffset.x += ((kernelStep + 0.5f) * texelSize.x) * dir.x;
        uvOffset.y += ((kernelStep + 0.5f) * texelSize.y) * dir.y;
        weight = gauss1Dimantion(kernelStep, sigma) + gauss1Dimantion(kernelStep + 1, sigma);
        output += tex.Sample(smp, uvOffset) * weight;
        sum += weight;
    }

    output *= rcp(sum);

    return output;
}

static const float32_t2 kIndex[3][3] = {
    { { -1.0f, -1.0f }, { 0.0f, -1.0f }, { 1.0f, -1.0f } },
    { { -1.0f,  0.0f }, { 0.0f,  0.0f }, { 1.0f,  0.0f } },
    { { -1.0f,  1.0f }, { 0.0f,  1.0f }, { 1.0f,  1.0f } },
};

float32_t4 main(Output input) : SV_TARGET{
    float32_t w, h, levels;
    tex.GetDimensions(0, w,h,levels);
    float32_t2 texelSize = float32_t2(rcp(w), rcp(h));
    float32_t4 result = float32_t4(0.0f,0.0f,0.0f,0.0f);
    result = GaussianBlur(
        texelSize, 
        input.uv, 
        kGaussianBlurState.kernelSize, 
        kGaussianBlurState.sigma, 
        kGaussianBlurState.dir
        );
    return result;
}