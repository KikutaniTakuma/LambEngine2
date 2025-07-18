/// ======================
/// ==  コンセプト定義  ==
/// ======================


#pragma once
#include <type_traits>

namespace Lamb {
	template<class T>
	concept IsPtr = std::is_pointer_v<T>;

	template<class T>
	concept IsNotPtr = !std::is_pointer_v<T>;

	template<class T>
	concept IsReference = std::is_reference_v<T>;

	template<class T>
	concept IsNotReference = !std::is_reference_v<T>;

	template<class T>
	concept IsNotReferenceAndPtr = requires { IsNotPtr<T> && IsNotReference<T>; };

	template<typename T>
	concept IsNumber = requires{ std::is_integral_v<T>&& std::is_floating_point_v<T>; };

	template<class T>
	concept IsNotNumber = !IsNumber<T>;

	template<class T>
	concept IsInt = std::is_integral_v<T>;

	template<class T>
	concept IsNotInt = !std::is_integral_v<T>;

	template<class T>
	concept IsFloat = std::is_floating_point_v<T>;

	template<class T>
	concept IsNotFloat = !std::is_floating_point_v<T>;

	template<typename T>
	concept IsObject = std::is_object_v<T>;

	template<typename T>
	concept IsNotObject = !std::is_object_v<T>;
}

