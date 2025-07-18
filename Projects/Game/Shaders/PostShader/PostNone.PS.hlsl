#include "Post.hlsli"
#include "../OutputStructs.hlsli"

PixelShaderOutPut main(Output input) {
    PixelShaderOutPut output;
    output.color =  tex.Sample(smp, input.uv) * kColor.color;

    return output;
}