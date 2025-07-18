/// ===================
/// ==  Tonemap関係  ==
/// ===================


#include "Tonemap.h"
#include <cmath>

/// <summary>
/// 参考 : https://technorgb.blogspot.com/2018/02/hyperbola-tone-mapping.html
/// </summary>

float32_t3 Tonemap(const TonemapParams& tc, const float32_t3& x)
{
	float32_t3 toe = -tc.mToe.x / (x + tc.mToe.y) + tc.mToe.z;
	float32_t3 mid = tc.mMid.x * x + tc.mMid.y;
	float32_t3 shoulder = -tc.mShoulder.x / (x + tc.mShoulder.y) + tc.mShoulder.z;

	float32_t3 result = Vector3::Lerp(toe, mid, Vector3::Step(tc.mBx.x, x));
	result = Vector3::Lerp(result, shoulder, Vector3::Step(tc.mBx.y, x));
	return result;
}

TonemapParams PrepareTonemapParams(const float32_t2 p1, const float32_t2 p2, const float32_t2 p3)
{
	TonemapParams tc;

	constexpr float kEpsilon = 1e-5f;

	float denom = p2.x - p1.x;
	denom = std::abs(denom) > kEpsilon ? denom : kEpsilon;
	float slope = (p2.y - p1.y) / denom;

	{
		tc.mMid.x = slope;
		tc.mMid.y = p1.y - slope * p1.x;
	}

	{
		denom = p1.y - slope * p1.x;
		denom = std::abs(denom) > kEpsilon ? denom : kEpsilon;
		tc.mToe.x = slope * p1.x * p1.x * p1.y * p1.y / (denom * denom);
		tc.mToe.y = slope * p1.x * p1.x / denom;
		tc.mToe.z = p1.y * p1.y / denom;
	}

	{
		denom = slope * (p2.x - p3.x) - p2.y + p3.y;
		denom = std::abs(denom) > kEpsilon ? denom : kEpsilon;
		tc.mShoulder.x = slope * std::pow(p2.x - p3.x, 2.0f) * std::pow(p2.y - p3.y, 2.0f) / (denom * denom);
		tc.mShoulder.y = (slope * p2.x * (p3.x - p2.x) + p3.x * (p2.y - p3.y)) / denom;
		tc.mShoulder.z = (-p2.y * p2.y + p3.y * (slope * (p2.x - p3.x) + p2.y)) / denom;
	}

	tc.mBx = float32_t2(p1.x, p2.x);
	tc.mBy = float32_t2(p1.y, p2.y);

	return tc;
}

float32_t TonemapToe(const float32_t x, const float32_t2 p1, const float32_t2 p2)
{
	float32_t n = (p2.y - p1.y) / (p2.x - p1.x);

	float32_t a1 = std::pow(n * p1.x - p1.y, 2.0f) / (n * p1.x * p1.x * p1.y * p1.y);
	a1 = std::max(a1, 1e-5f);

	float32_t t1 = std::floor(p1.y / (n * p1.x)) - 0.5f;
	float32_t s1 = std::signbit(t1) ? -1.0f : 1.0f;

	float32_t xOffset = s1 * std::sqrt(1.0f / (a1 * n)) - p1.x;
	float32_t yOffset = s1 * std::sqrt(n / a1) + p1.y;

	return -(1.0f / (a1 * (x + xOffset))) + yOffset;
}

float32_t TonemapSholder(const float32_t x, const float32_t2 p1, const float32_t2 p2, const float32_t2 p3)
{
	float32_t pl = p3.y - p2.y;
	float32_t n = (p2.y - p1.y) / (p2.x - p1.x);

	float32_t a2 = std::pow(n * (p3.x - p2.x) - pl, 2.0f) / (n * std::pow(p3.x - p2.x, 2.0f) * pl * pl);
	a2 = std::max(a2, 1e-5f);

	float32_t t2 = std::floor(n * ((p3.x - p2.x) / (p3.y - p2.y))) - 0.5f;
	float32_t s2 = std::signbit(t2) ? -1.0f : 1.0f;

	float32_t xOffset = s2 * std::sqrt(1 / (a2 * n)) - p2.x;
	float32_t yOffset = s2 * std::sqrt(n / a2) + p2.y;

	return -1.0f / (a2 * (x + xOffset)) + yOffset;
}
