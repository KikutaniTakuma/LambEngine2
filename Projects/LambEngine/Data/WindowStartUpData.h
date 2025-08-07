#pragma once
#include <string>
#include <cstdint>
#include "Data.h"

namespace Lamb {
	struct WindowStartUpData : public Data {
		std::wstring windowTitle = L"LambEngine";
		int32_t width = 1280;
		int32_t height = 720;
		bool isFullscreen = false;
	};


	void to_json(nlohmann::json& json, const WindowStartUpData& windata) {
		json["windowTitle"] = ConvertString(windata.windowTitle);
		json["width"] = windata.width;
		json["height"] = windata.height;
		json["isFullscreen"] = windata.isFullscreen;
	}
	void from_json(const nlohmann::json& json, WindowStartUpData& windata) {
		windata.windowTitle = ConvertString(json["windowTitle"].get<std::string>());
		windata.width = json["width"].get<int>();
		windata.height = json["height"].get<int>();
		windata.isFullscreen = json["isFullscreen"].get<bool>();
	}
}