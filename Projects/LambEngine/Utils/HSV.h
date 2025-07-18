#pragma once
#include "Math/Vector3.h"
#include "Math/MathCommon.h"
#include <algorithm>


/// <summary>
/// RGBからHSVへの変換
/// </summary>
/// <param name="rgb"></param>
/// <returns></returns>
inline float32_t3 RGBToHSV(float32_t3 rgb) {
    float32_t r = rgb.x;
    float32_t g = rgb.y;
    float32_t b = rgb.z;

    float32_t maxVal = std::max(r, std::max(g, b));
    float32_t minVal = std::min(r, std::min(g, b));
    float32_t delta = maxVal - minVal;

    float32_t h = 0.0f;
    float32_t s = (maxVal == 0.0f) ? 0.0f : delta / maxVal;
    float32_t v = maxVal;

    if (delta != 0.0f) {
        if (maxVal == r) {
            h = 60.0f * ((g - b) / delta);
            h += 360.0f * ((g < h) ? 1.0f : 0.0f);
        }
        else if (maxVal == g) {
            h = 60.0f * ((b - r) / delta + 2.0f);
        }
        else {
            h = 60.0f * ((r - g) / delta + 4.0f);
        }
    }

    return float32_t3(h, s, v);
}

/// <summary>
/// HSVからRGBへの変換
/// </summary>
/// <param name="hsv"></param>
/// <returns></returns>
inline float32_t3 HSVToRGB(float32_t3 hsv) {
    float32_t h = hsv.x;
    float32_t s = hsv.y;
    float32_t v = hsv.z;

    float32_t c = v * s;
    float32_t x = c * (1.0f - abs(fmod(h / 60.0f, 2.0f) - 1.0f));
    float32_t m = v - c;

    float32_t3 rgb;

    if (0.0f <= h && h < 60.0f) {
        rgb = float32_t3(c, x, 0);
    }
    else if (60.0f <= h && h < 120.0f) {
        rgb = float32_t3(x, c, 0);
    }
    else if (120.0f <= h && h < 180.0f) {
        rgb = float32_t3(0, c, x);
    }
    else if (180.0f <= h && h < 240.0f) {
        rgb = float32_t3(0, x, c);
    }
    else if (240.0f <= h && h < 300.0f) {
        rgb = float32_t3(x, 0, c);
    }
    else {
        rgb = float32_t3(c, 0, x);
    }

    rgb.x += m;
    rgb.y += m;
    rgb.z += m;

    return rgb;
}