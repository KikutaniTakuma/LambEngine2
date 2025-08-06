#pragma once
#include "Utils/Concepts.h"

namespace Lamb {
	/// <summary>
	/// 安全にdeleteするため
	/// </summary>
	/// <typeparam name="T"></typeparam>
	/// <param name="ptr"></param>
	template<IsPtr T>
	void SafeDelete(T& ptr) {
		delete ptr;
		ptr = nullptr;
	}
}