#pragma once
#include <string>

namespace Lamb {
	/// 
	/// string to wstring
	/// 
	std::wstring ConvertString(const std::string& msg);

	/// 
	/// wstring to string
	/// 
	std::string ConvertString(const std::wstring& msg);
}