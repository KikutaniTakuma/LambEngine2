Texture2D<float32_t4> tex : register(t0);
SamplerState smp : register(s0);

struct Color{
    float32_t4 color;
};

ConstantBuffer<Color> kColor : register(b0);

struct Output{
    float32_t4 svPos : SV_POSITION;
    float32_t2 uv : TEXCOORD;
};