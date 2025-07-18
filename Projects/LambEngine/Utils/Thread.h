#pragma once
#include <mutex>
#include <thread>
#include <condition_variable>
#include <functional>
#include <memory>

namespace Lamb {
	/// <summary>
	/// ゲームループ内で別threadを使うためのクラス
	/// </summary>
	class Thread {
	public:
		static uint32_t GetCurretnThreadNum() {
			return currentThreadNum_;
		}
		static uint32_t GetHardwareThread() {
			return kHardwareThread_;
		}

	private:
		static uint32_t currentThreadNum_;
		static uint32_t kHardwareThread_;

	public:
		Thread() = default;
		Thread(const Thread&) = delete;
		Thread(Thread&&) = delete;
		~Thread();

	public:
		Thread& operator=(const Thread&) = delete;
		Thread& operator=(Thread&&) = delete;

	public:
		/// <summary>
		/// thread処理開始
		/// </summary>
		void Notify();

		/// <summary>
		/// thread作成
		/// </summary>
		void Create(
			const std::function<void(void)>& userProcess,
			const std::function<bool(void)>& waitProcess,
			const std::function<bool(void)>& restartProcess
		);

		bool IsWait() const {
			return isWait_;
		}

	private:
		std::unique_ptr<std::thread> thraed_;
		std::unique_ptr<std::mutex> mtx_;
		std::unique_ptr<std::condition_variable> condition_;

		// threadで行う処理
		std::unique_ptr<std::function<void(void)>> threadProcess_;
		// threadProsess内で行う処理。
		std::unique_ptr<std::function<void(void)>> userProcess_;
		// waitに入るための関数(wait状態にするか否か)
		std::unique_ptr<std::function<bool(void)>> waitProcess_;
		// waitから起きるための関数(notify_allで起きるために必要)
		std::unique_ptr<std::function<bool(void)>> restartProcess_;

		bool exit_ = false;
		bool isWait_ = false;
	};
};