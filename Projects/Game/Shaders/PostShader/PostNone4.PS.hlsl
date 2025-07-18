#include "Post.hlsli"
#include "../OutputStructs.hlsli"

PixelShaderOutPut3 main(Output input) {
    PixelShaderOutPut3 output;
    output.color0 =  tex.Sample(smp, input.uv) * kColor.color;
    output.color1 = output.color0;
    output.color2 = output.color0;
    output.color3 = output.color0;

    return output;
}