#include "PerlinNoise.hlsli"

float32_t CreateNoiseDdy(float32_t2 uv, float32_t2 vec, float32_t densityScale)
{
    float32_t density = 20.0f * densityScale;
    
    float32_t pn = FractalSumNnoise(density, uv + vec);
    float32_t pn2 = FractalSumNnoise(density, uv - vec);
    uv.x *= -1.0f;
    float32_t pn3 = FractalSumNnoise(density, uv + vec);
    uv.x *= -1.0f;
    uv.y *= -1.0f;
    float32_t pn4 = FractalSumNnoise(density, uv + vec);
    
    float32_t noise = lerp((pn * 0.1f), (pn2 * 0.1f), 3.0f);
    float32_t noise2 = lerp((pn3 * 0.1f), (pn4 * 0.1f), 3.0f);
    
    return ddy(noise + noise2);
}

float32_t3 CreateNormal(float32_t2 uv, float32_t2 vec, float32_t densityScale)
{
    float32_t heightScale = 25.0f;
    
    float32_t right  = CreateNoiseDdy(float32_t2(uv.x + 1.0f, uv.y       ), vec, densityScale) * heightScale;
    float32_t left   = CreateNoiseDdy(float32_t2(uv.x - 1.0f, uv.y       ), vec, densityScale) * heightScale;
    float32_t up     = CreateNoiseDdy(float32_t2(uv.x,        uv.y + 1.0f), vec, densityScale) * heightScale;
    float32_t bottom = CreateNoiseDdy(float32_t2(uv.x,        uv.y - 1.0f), vec, densityScale) * heightScale;
    
    float32_t dfx = right - left;
    float32_t dfy = up - bottom;
    
    float32_t3 n = normalize(float32_t3(-dfx, -dfy, 2.0f));
    
    float32_t a = 0.01f;
    n = (n / a) * 0.5f;
    
    n = (n * 2.0f) - 1.0f;
   
    return normalize(n);
}