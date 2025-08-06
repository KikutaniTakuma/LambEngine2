#pragma once
#include <vector>
#include <filesystem>
#include "json.hpp"

/// <summary>
/// ファイル関係の関数やクラス
/// </summary>
namespace Lamb {
	/// <summary>
	/// 特定のフォルダの特定の拡張子のファイルをすべて探して返す
	/// </summary>
	/// <param name="directoryName">ディレクトリ名(例 : "./Resources/")</param>
	/// <param name="extension">拡張子(例 : ".png")</param>
	/// <returns>ファイルのパスを格納したコンテナ</returns>
	std::vector<std::filesystem::path> GetFilePathFormDir(
		const std::filesystem::path& directoryName,
		const std::filesystem::path& extension
	);

	/// <summary>
	/// Csvローダー
	/// </summary>
	/// <param name="fileNmae">csvのファイル名</param>
	/// <returns>ロードしたcsvの二次元配列</returns>
	std::vector<std::vector<int32_t>> LoadCsv(const std::string& fileNmae);

	/// <summary>
	/// jsonファイルのローダー
	/// </summary>
	/// <param name="fileName">読み込むファイルの名前</param>
	/// <returns>解凍したjsonファイル</returns>
	nlohmann::json LoadJson(const std::filesystem::path& fileName);

	/// <summary>
	/// 開くファイルをダイアログを開いて選択する
	/// </summary>
	/// <returns>current directoryから相対パス。キャンセルすると空の文字列が返る</returns>
	std::wstring ShowSaveFileDialog();

	/// <summary>
	/// 保存するファイルをダイアログを開いて選択する
	/// </summary>
	/// <returns>current directoryから相対パス。キャンセルすると空の文字列が返る</returns>
	std::wstring ShowOpenFileDialog();

	/// <summary>
	/// 開くファイルをダイアログを開いて選択する(デフォルトでjson)
	/// </summary>
	/// <returns>current directoryから相対パス。キャンセルすると空の文字列が返る</returns>
	std::wstring ShowSaveFileDialogJson();

	/// <summary>
	/// 保存するファイルをダイアログを開いて選択する(デフォルトでjson)
	/// </summary>
	/// <returns>current directoryから相対パス。キャンセルすると空の文字列が返る</returns>
	std::wstring ShowOpenFileDialogJson();

	/// <summary>
	/// ダイアログから削除
	/// </summary>
	void ShowDeleteFileDialog();
}