#include "Post.hlsli"

float32_t4 main(Output input) : SV_TARGET{
    float32_t4 color;

    color = tex.Sample(smp, input.uv);
    
    float32_t t = dot(color.xyz, float32_t3(0.299f, 0.587f, 0.144f));

    color.r = t;
    color.g = t;
    color.b = t;

    return color;
}