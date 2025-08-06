#include "FileUtils.h"
#include "Error.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <Windows.h>
#undef max
#undef min

#include <commdlg.h>
#pragma comment(lib, "comdlg32.lib")


namespace Lamb {
	std::vector<std::filesystem::path> GetFilePathFormDir(
		const std::filesystem::path& directoryName,
		const std::filesystem::path& extension
	) {
		std::vector<std::filesystem::path> result;
		if (not std::filesystem::exists(directoryName)) {
			return result;
		}

		std::filesystem::directory_iterator directory{ directoryName };

		// ディレクトリ内を探索
		for (const auto& entry : directory) {
			const auto& path = entry.path();
			// もし拡張子がなかったら
			if (!path.has_extension()) {
				std::vector<std::filesystem::path> files;
				// その中にあるファイルをすべて探索(再帰的に探す)
				files = GetFilePathFormDir(path, extension);

				// コンテナに追加
				for (auto& i : files) {
					result.push_back(i);
				}
			}

			// 特定の拡張子を持つファイルだった場合コンテナに追加
			else if (path.extension() == extension) {
				result.push_back(path);
			}
		}

		return result;
	}

	std::vector<std::vector<int32_t>> LoadCsv(const std::string& fileName) {
		std::vector<std::vector<int32_t>> result;

		if (!(std::filesystem::path(fileName).extension() == ".csv")) {
			throw Lamb::Error::Code<Lamb::Error::Function>("This file is not csv -> " + fileName, ErrorPlace);
		}
		if (not std::filesystem::exists(fileName)) {
			throw Lamb::Error::Code<Lamb::Error::Function>("This file is not exists -> " + fileName, ErrorPlace);
		}

		std::ifstream file{ fileName };

		std::string line;
		while (std::getline(file, line))
		{
			std::stringstream sLine{ line };
			std::string stringBuf;

			result.push_back(std::vector<int32_t>());

			while (std::getline(sLine, stringBuf, ',')) {
				if (std::any_of(stringBuf.begin(), stringBuf.end(), isdigit)) {
					result.back().push_back(std::atoi(stringBuf.c_str()));
				}
			}
		}

		return result;
	}

	nlohmann::json LoadJson(const std::filesystem::path& fileName)
	{
		if (not std::filesystem::exists(fileName)) {
			throw Lamb::Error::Code<Lamb::Error::Function>("This file is not exists -> " + fileName.string(), ErrorPlace);
		}
		if (not (fileName.extension() == ".json")) {
			throw Lamb::Error::Code<Lamb::Error::Function>("This file is not json -> " + fileName.string(), ErrorPlace);
		}

		std::ifstream file;

		file.open(fileName);

		if (not file.is_open()) {
			throw Lamb::Error::Code<Lamb::Error::Function>("This file can not open -> " + fileName.string(), ErrorPlace);
		}

		nlohmann::json result;

		file >> result;

		return result;
	}

	std::wstring ShowSaveFileDialog()
	{
		wchar_t filename[MAX_PATH] = L"";
		wchar_t originalDir[MAX_PATH] = L"";


		GetCurrentDirectoryW(MAX_PATH, originalDir);

		OPENFILENAMEW ofn = {};
		ofn.lStructSize = sizeof(ofn);
		ofn.hwndOwner = nullptr;
		ofn.lpstrFilter = L"All Files (*.*)\0*.*\0";
		ofn.lpstrFile = filename;
		ofn.nMaxFile = MAX_PATH;
		ofn.Flags = OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST;
		ofn.lpstrDefExt = L"txt";
		ofn.lpstrInitialDir = originalDir;

		std::wstring result = L"";

		if (GetSaveFileNameW(&ofn)) {
			std::filesystem::path fileNamePath = filename;

			fileNamePath = std::filesystem::absolute(fileNamePath);

			result = ("./" / std::filesystem::relative(fileNamePath, "../")).generic_wstring();
		}

		SetCurrentDirectoryW(originalDir);

		return result;
	}

	std::wstring ShowOpenFileDialog()
	{
		wchar_t filename[MAX_PATH] = L"";
		wchar_t originalDir[MAX_PATH] = L"";

		GetCurrentDirectoryW(MAX_PATH, originalDir);


		OPENFILENAMEW ofn = {};
		ofn.lStructSize = sizeof(ofn);
		ofn.hwndOwner = nullptr;
		ofn.lpstrFilter = L"All Files (*.*)\0*.*\0";
		ofn.lpstrFile = filename;
		ofn.nMaxFile = MAX_PATH;
		ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
		ofn.lpstrInitialDir = originalDir;

		std::wstring result = L"";

		if (GetOpenFileNameW(&ofn)) {
			std::filesystem::path fileNamePath = filename;

			fileNamePath = std::filesystem::absolute(fileNamePath);

			result = ("./" / std::filesystem::relative(fileNamePath.filename(), "../")).generic_wstring();
		}

		SetCurrentDirectoryW(originalDir);

		return result;
	}

	std::wstring ShowSaveFileDialogJson()
	{
		wchar_t filename[MAX_PATH] = L"";
		wchar_t originalDir[MAX_PATH] = L"";


		GetCurrentDirectoryW(MAX_PATH, originalDir);

		OPENFILENAMEW ofn = {};
		ofn.lStructSize = sizeof(ofn);
		ofn.hwndOwner = nullptr;
		ofn.lpstrFilter = L"JSON Files (*.json)\0*.json\0All Files (*.*)\0*.*\0";
		ofn.lpstrFile = filename;
		ofn.nMaxFile = MAX_PATH;
		ofn.Flags = OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST;
		ofn.lpstrDefExt = L"json";
		ofn.lpstrInitialDir = originalDir;

		std::wstring result = L"";

		if (GetSaveFileNameW(&ofn)) {
			std::filesystem::path fileNamePath = filename;

			fileNamePath = std::filesystem::absolute(fileNamePath);

			result = ("./" / std::filesystem::relative(fileNamePath, "../")).generic_wstring();
		}

		SetCurrentDirectoryW(originalDir);

		return result;
	}
	std::wstring ShowOpenFileDialogJson()
	{
		wchar_t filename[MAX_PATH] = L"";
		wchar_t originalDir[MAX_PATH] = L"";

		GetCurrentDirectoryW(MAX_PATH, originalDir);


		OPENFILENAMEW ofn = {};
		ofn.lStructSize = sizeof(ofn);
		ofn.hwndOwner = nullptr;
		ofn.lpstrFilter = L"JSON Files (*.json)\0*.json\0All Files (*.*)\0*.*\0";
		ofn.lpstrFile = filename;
		ofn.nMaxFile = MAX_PATH;
		ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
		ofn.lpstrInitialDir = originalDir;

		std::wstring result = L"";

		if (GetOpenFileNameW(&ofn)) {
			std::filesystem::path fileNamePath = filename;

			fileNamePath = std::filesystem::absolute(fileNamePath);

			result = ("./" / std::filesystem::relative(fileNamePath.filename(), "../")).generic_wstring();
		}

		SetCurrentDirectoryW(originalDir);

		return result;
	}
	void ShowDeleteFileDialog()
	{
		wchar_t filename[MAX_PATH] = L"";
		wchar_t originalDir[MAX_PATH] = L"";

		GetCurrentDirectoryW(MAX_PATH, originalDir);

		OPENFILENAMEW ofn = {};
		ofn.lStructSize = sizeof(ofn);
		ofn.hwndOwner = nullptr;
		ofn.lpstrFilter = L"All Files (*.*)\0*.*\0";
		ofn.lpstrFile = filename;
		ofn.nMaxFile = MAX_PATH;
		ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
		ofn.lpstrInitialDir = originalDir;

		if (GetOpenFileNameW(&ofn)) {
			std::filesystem::path fileNamePath = filename;

			fileNamePath = std::filesystem::absolute(fileNamePath);

			std::wstring filePath = fileNamePath.generic_wstring();

			std::wstring currentFilePath = ("./" / std::filesystem::relative(fileNamePath.filename(), "../")).generic_wstring();

			if (MessageBoxW(nullptr, L"このファイルを削除しますか？", currentFilePath.c_str(), MB_YESNO) == IDYES) {
				if (!std::filesystem::remove(filePath)) {
					MessageBoxW(nullptr, L"削除に失敗しました", L"エラー", MB_OK | MB_ICONERROR);
				}
			}
		}

		SetCurrentDirectoryW(originalDir);
	}
}