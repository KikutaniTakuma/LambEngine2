#pragma once
#include "Math/MathCommon.h"
#include "Math/Vector2.h"

namespace Lamb {
    float32_t Random(float32_t2 uv);

    float32_t2 RandomVector(float32_t2 fact);

    float32_t Perlin(float32_t density, float32_t2 uv);

    float32_t FractalSumNnoise(float32_t density, float32_t2 uv);

    float32_t CreateNoise(float32_t2 uv, float32_t2 vec, float32_t densityScale);
}