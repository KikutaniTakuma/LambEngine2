#include "PerlinNoise.h"

namespace Lamb {

    float32_t Random(float32_t2 uv) {
        return Lamb::Frac(sin(uv.Dot(float32_t2(12.9898f, 78.233f))) * 43758.5453f);
    }

    float32_t2 RandomVector(float32_t2 fact) {
        float32_t2 result = float32_t2(
            Lamb::Frac(std::sin(fact.Dot(float32_t2(127.1f, 311.7f))) * 43758.5453123f) * 2.0f - 1.0f,
            Lamb::Frac(std::sin(fact.Dot(float32_t2(269.5f, 183.3f))) * 43758.5453123f) * 2.0f - 1.0f
        );

        return result;
    }

    float32_t Perlin(float32_t density, float32_t2 uv)
    {
        float32_t2 uvFloor = float32_t2::Floor(uv * density);
        float32_t2 uvFrac = float32_t2::Frac(uv * density);

        float32_t2 v00 = RandomVector(uvFloor + float32_t2(0.0f, 0.0f));
        float32_t2 v01 = RandomVector(uvFloor + float32_t2(0.0f, 1.0f));
        float32_t2 v10 = RandomVector(uvFloor + float32_t2(1.0f, 0.0f));
        float32_t2 v11 = RandomVector(uvFloor + float32_t2(1.0f, 1.0f));

        float32_t c00 = v00.Dot(uvFrac - float32_t2(0.0f, 0.0f));
        float32_t c01 = v01.Dot(uvFrac - float32_t2(0.0f, 1.0f));
        float32_t c10 = v10.Dot(uvFrac - float32_t2(1.0f, 0.0f));
        float32_t c11 = v11.Dot(uvFrac - float32_t2(1.0f, 1.0f));

        float32_t2 u = uvFrac * uvFrac * (3.0f - 2.0f * uvFrac);

        float32_t v0010 = std::lerp(c00, c10, u.x);
        float32_t v0111 = std::lerp(c01, c11, u.x);

        float32_t n = (std::lerp(v0010, v0111, u.y) * 0.5f) + 0.5f;

        return n;
    }

    float32_t FractalSumNnoise(float32_t density, float32_t2 uv)
    {
        float32_t fn = 0.0f;
        fn += Perlin(density * 1.0f, uv) * 1.0f / 2.0f;
        fn += Perlin(density * 2.0f, uv) * 1.0f / 4.0f;
        fn += Perlin(density * 4.0f, uv) * 1.0f / 8.0f;
        fn += Perlin(density * 8.0f, uv) * 1.0f / 16.0f;

        return fn;
    }

    float32_t CreateNoise(float32_t2 uv, float32_t2 vec, float32_t densityScale)
    {
        float32_t density = 20.0f * densityScale;

        float32_t pn = FractalSumNnoise(density * 2.0f, uv + vec);
        float32_t pn2 = FractalSumNnoise(density, uv - vec);
        uv.x *= -1.0f;
        float32_t pn3 = FractalSumNnoise(density * 2.0f, uv + vec);
        uv.x *= -1.0f;
        uv.y *= -1.0f;
        float32_t pn4 = FractalSumNnoise(density, uv + vec);

        float32_t noise = std::lerp((pn * 0.1f), (pn2 * 0.08f), 3.0f);
        float32_t noise2 = std::lerp((pn3 * 0.1f), (pn4 * 0.08f), 3.0f);

        return std::lerp(noise, noise2, 0.5f);
    }
}