/// ================
/// ==  数学関数  ==
/// ================


#include "MathCommon.h"
#include <cmath>

namespace Lamb {
    namespace Math {
        float LengthSQ(const std::initializer_list<float>& data)
        {
            float result = 0.0f;
            for (const auto& i : data) {
                result += i * i;
            }
            return result;
        }

        float Length(const std::initializer_list<float>& data)
        {
            return std::sqrt(LengthSQ(data));
        }
        float LerpShortAngle(float a, float b, float t) {
            // 角度差分を求める
            float diff = b - a;
            // 角度[-2PI,+2PI]に補正
            diff = std::fmod(diff, 2.0f * std::numbers::pi_v<float>);
            // 角度[-PI,+PI]に補正
            if (diff > std::numbers::pi_v<float>) {
                diff -= 2.0f * std::numbers::pi_v<float>;
            }
            else if (diff < -std::numbers::pi_v<float>) {
                diff += 2.0f * std::numbers::pi_v<float>;
            }
            return std::lerp(a, a + diff, t);
        }
    }
}