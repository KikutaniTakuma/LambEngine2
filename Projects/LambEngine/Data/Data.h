#pragma once
#include <type_traits>
#include <nlohmann/json.hpp>
#include "Utils/ObjectNameToString.h"
#include "Utils/ConvertString.h"

namespace Lamb {
	/// <summary>
	/// jsonに出力するための基底クラス
	/// </summary>
	struct Data {
		virtual ~Data() = default;
	};

	/// <summary>
	/// Dataクラスを継承しているか
	/// </summary>
	template<class T>
	concept IsDataBased = std::is_base_of_v<Data, T> && requires(T a) {
		{ a.~T() } -> std::convertible_to<void>;
	};

	/// <summary>
	/// jsonに保存
	/// </summary>
	/// <param name="json">json</param>
	/// <param name="data">それぞれのデータ型</param>
	template<IsDataBased T>
	void DataToJson(nlohmann::json& json, const T& data) {
		json[ToString<T>()] = data;
	}
	/// <summary>
	/// jsonからロード
	/// </summary>
	/// <param name="json">json</param>
	/// <param name="data">それぞれのデータ型</param>
	template<IsDataBased T>
	void JsonToData(const nlohmann::json& json, T& data) {
		data = json[ToString<T>()].get<T>();
	}
}

/*
Lamb::Dataを継承しているなら以下の関数は実装されていることを保証しなければならない(Must)
json用にシリアライズするための関数を作ってください
ex:

	void to_json(nlohmann::json& json, const YourData& data) {
		json["???"] = data.???;
	}
	void from_json(const nlohmann::json& json, YourData& data) {
		data.??? = json["???"].get<???>();
	}


*/