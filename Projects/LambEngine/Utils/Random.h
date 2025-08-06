#pragma once

#include <random>
#include <algorithm>
#include <memory>
#include "Math/Vector2.h"
#include "Math/Vector3.h"
#include "Math/Vector4.h"

namespace Lamb {

	/// <summary>
	/// シードとランダムエンジンの生成管理
	/// </summary>
	class RandomGenereator {
	private:
		RandomGenereator();
		~RandomGenereator() = default;
		RandomGenereator(const RandomGenereator&) = delete;
		RandomGenereator(RandomGenereator&&) = delete;

		RandomGenereator& operator=(const RandomGenereator&) = delete;
		RandomGenereator& operator=(RandomGenereator&&) = delete;

	public:
		static RandomGenereator* const GetInstance();

	public:
		uint32_t GetSeed() const;

		std::mt19937_64& GetGenerator();

	private:
		const uint32_t seed_ = std::random_device()();
		std::unique_ptr<std::mt19937_64> generator_;
	};

	extern RandomGenereator* const generator;

	/// <summary>
	/// 整数型のランダム関数
	/// </summary>
	/// <typeparam name="T">32bit以下の整数型のみサポート</typeparam>
	/// <param name="min">最小値</param>
	/// <param name="max">最大値</param>
	/// <returns>ランダムな値</returns>
	template<IsInt T> requires requires { sizeof(T) <= sizeof(long); }
	T Random(T min, T max) {
		if (max < min) {
			std::swap(min, max);
		}

		std::uniform_int_distribution<T> dist{ min, max };

		return static_cast<T>(dist(generator->GetGenerator()));
	}

	/// <summary>
	/// 最小値から最大値までで一様分布で乱数を生成
	/// </summary>
	/// <param name="min">最小値</param>
	/// <param name="max">最大値</param>
	/// <returns>乱数</returns>
	float Random(float min, float max);
	/// <summary>
	/// 最小値から最大値までで一様分布で乱数を生成
	/// </summary>
	/// <param name="min">最小値</param>
	/// <param name="max">最大値</param>
	/// <returns>乱数</returns>
	double Random(double min, double max);

	/// <summary>
	/// 最小値から最大値までで一様分布で乱数を生成
	/// </summary>
	/// <param name="min">最小値</param>
	/// <param name="max">最大値</param>
	/// <returns>乱数</returns>
	Vector2 Random(const Vector2& min, const Vector2& max);
	/// <summary>
	/// 最小値から最大値までで一様分布で乱数を生成
	/// </summary>
	/// <param name="min">最小値</param>
	/// <param name="max">最大値</param>
	/// <returns>乱数</returns>
	Vector3 Random(const Vector3& min, const Vector3& max);

	/// <summary>
	/// 色のランダム
	/// </summary>
	/// <param name="min">最小値</param>
	/// <param name="max">最大値</param>
	/// <returns>乱数</returns>
	uint32_t RandomColor(uint32_t min, uint32_t max);
	/// <summary>
	/// 色のランダム
	/// </summary>
	/// <param name="min">最小値</param>
	/// <param name="max">最大値</param>
	/// <returns>乱数</returns>
	Vector4 RandomColor(const Vector4& min, const Vector4& max);
}