#include "Post.hlsli"

float32_t4 main(Output input) : SV_TARGET{
    float32_t4 color;

    color = tex.Sample(smp, input.uv);

    float32_t2 correct = input.uv * (1.0f - input.uv.yx);

    float32_t vignette = correct.x * correct.y * 16.0f;

    vignette = saturate(pow(vignette, 0.8f));

    color.rgb *= vignette;

    return color;
}