/// ================
/// ==  数学関数  ==
/// ================


#pragma once
#include <numbers>
#include <limits>
#include <initializer_list>
#include <cmath>

#if _HAS_CXX23
#include <stdfloat>
#endif

#include "Utils/Concepts.h"

namespace Lamb {
	namespace Math {
		template<std::floating_point Floating>
		constexpr Floating toRadian = static_cast<Floating>(std::numbers::pi_v<double> / 180.0);

		template<std::floating_point Floating>
		constexpr Floating toDegree = static_cast <Floating>(180.0 / std::numbers::pi_v<double>);

		template<std::floating_point Floating>
		constexpr Floating ToRadian(Floating degree) {
			return std::fmod(degree, 360.0f) * toRadian<Floating>;
		}
		template<std::floating_point Floating>
		constexpr Floating ToDegree(Floating radian) {
			return std::fmod(radian, 2.0f * std::numbers::pi_v<Floating>) * toDegree<Floating>;
		}

		float LengthSQ(const std::initializer_list<float>& data);
		float Length(const std::initializer_list<float>& data);

		float LerpShortAngle(float a, float b, float t);
	}

	template<Lamb::IsNumber T>
	bool Between(const T& num, const T& min, const T& max) {
		return min <= num && num <= max;
	}

	template<Lamb::IsNumber T>
	bool Step(const T& a, const T& x) {
		return x < a ? static_cast<T>(0) : static_cast<T>(1);
	}

	template<Lamb::IsFloat T>
	float Frac(T a) {
		return a - std::floor(a);
	}
}

inline uint16_t operator""_u16(size_t i) {
	return static_cast<uint16_t>(i);
}

inline uint32_t operator""_u32(size_t i) {
	return static_cast<uint32_t>(i);
}

inline uint64_t operator""_u64(size_t i) {
	return i;
}
inline size_t operator""_z(size_t i) {
	return i;
}

inline int16_t operator""_16(size_t i) {
	return static_cast<uint16_t>(i);
}

inline int32_t operator""_32(size_t i) {
	return static_cast<uint32_t>(i);
}

inline int64_t operator""_64(size_t i) {
	return i;
}

#if !_HAS_CXX23
using float32_t = float;
using float64_t = double;
#endif // !_HAS_CXX23

inline float operator""_f32(long double i) {
	return static_cast<float>(i);
}

inline double operator""_f64(long double i) {
	return static_cast<double>(i);
}
