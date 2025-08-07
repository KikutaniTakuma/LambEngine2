#pragma once

/// <summary>
/// フレームワーク
/// </summary>
namespace Lamb{
	class Framework {
	private:
		Framework(const Framework&) = delete;
		Framework(Framework&&) = delete;

		Framework& operator=(const Framework&) = delete;
		Framework& operator=(Framework&&) = delete;

	public:
		Framework() = default;
		~Framework() = default;

	public:
		void Execution();

	private:
		void InitializeLibrary_();
		void FinalizeLibrary_();


	};
}