#include "FileUtils.h"

#include "Error/Error.h"
#include <fstream>
#include <sstream>
#include <algorithm>

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
}