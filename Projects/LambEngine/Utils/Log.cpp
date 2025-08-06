#include "Utils/Log.h"

#include "Math/Vector2.h"
#include "Math/Vector3.h"
#include "Math/Vector4.h"
#include "Math/Quaternion.h"

namespace Lamb {
	std::string TimeToString(std::chrono::milliseconds ms) {
		// 1秒、1分、1時間のミリ秒
		auto h = std::chrono::duration_cast<std::chrono::hours>(ms);
		ms -= h;
		auto m = std::chrono::duration_cast<std::chrono::minutes>(ms);
		ms -= m;
		auto s = std::chrono::duration_cast<std::chrono::seconds>(ms);
		ms -= s;

		// 文字列フォーマット
		return std::to_string(h.count()) + "h " +
			std::to_string(m.count()) + "m " +
			std::to_string(s.count()) + "s " +
			std::to_string(ms.count()) + "ms";
	}

	void DebugLog(const std::string& text) {
		OutputDebugStringA((text + "\n").c_str());
	}

	void DebugLog(const std::string& text, const Vector2& vec) {
		OutputDebugStringA((text + std::string{ " : " } + std::format("{}, {}", vec.x, vec.y) + "\n").c_str());
	}

	void DebugLog(const std::string& text, const Vector3& vec) {
		OutputDebugStringA((text + std::string{ " : " } + std::format("{}, {}, {}", vec.x, vec.y, vec.z) + "\n").c_str());
	}

	void DebugLog(const std::string& text, const Vector4& vec) {
		OutputDebugStringA((text + std::string{ " : " } + std::format("{}, {}, {}, {}", vec.vec.x, vec.vec.y, vec.vec.z, vec.vec.w) + "\n").c_str());
	}

	void DebugLog(const std::string& text, const Quaternion& quaternion) {
		DebugLog(text, quaternion.vector4);
	}

	std::string NowTime() {
		auto now = std::chrono::system_clock::now();
		auto nowSec = std::chrono::floor<std::chrono::seconds>(now);
		std::chrono::zoned_time zt{ "Asia/Tokyo", nowSec };

		return std::format("{:%Y/%m/%d %H:%M:%S}", zt);
	}
}