#pragma once
#include <xhash>

namespace Lamb {

	/// <summary>
	/// ハッシュ値を生成する関数
	/// </summary>
	/// <typeparam name="T">組み込み型、一部のSTLのみ対応</typeparam>
	/// <param name="v">ハッシュ値にしたい値</param>
	/// <returns>ハッシュ値</returns>
	template<class T>
	size_t CreateHash(const T& v) {
		return std::hash<T>()(v);
	}
}