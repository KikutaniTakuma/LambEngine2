#include "Thread.h"
#include "Engine/Engine.h"
#include "Utils/ExecutionLog.h"
#include "Error/Error.h"


namespace Lamb {

	uint32_t Thread::currentThreadNum_ = 0;
	uint32_t Thread::kHardwareThread_ = std::thread::hardware_concurrency();

	Thread::~Thread() {
		exit_ = true;
		condition_->notify_all();
		if (thraed_->joinable()) {
			thraed_->join();
		}
	}

	void Thread::Notify() {
		condition_->notify_all();
	}

	void Thread::Create(
		const std::function<void(void)>& userProcess,
		const std::function<bool(void)>& waitProcess,
		const std::function<bool(void)>& restartProcess
	) {
		if (kHardwareThread_ <= currentThreadNum_) {
			throw Error::Code<Thread>("over hardware thread number", ErrorPlace);
		}
		if (thraed_ and thraed_->joinable()) {
			throw Error::Code<Thread>("Already created", ErrorPlace);
		}

		mtx_ = std::make_unique<std::mutex>();
		condition_ = std::make_unique<std::condition_variable>();

		userProcess_ = std::make_unique<std::function<void(void)>>(userProcess);
		waitProcess_ = std::make_unique<std::function<bool(void)>>(waitProcess);
		restartProcess_ = std::make_unique<std::function<bool(void)>>(restartProcess);

		threadProcess_ = std::make_unique<std::function<void(void)>>(
			[this]() {
				HRESULT hr = CoInitializeEx(0, COINIT_MULTITHREADED);
				if (SUCCEEDED(hr)) {
					AddLog("CoInitializeEx succeeded");
				}
				else {
					throw Error::Code<Thread>("CoInitializeEx failed", ErrorPlace);
				}

				std::unique_lock<std::mutex> uniqueLock(*mtx_);

				auto& waitProcess = *waitProcess_;
				auto& restartProcess = *restartProcess_;
				auto& userProcess = *userProcess_;
				auto& condition = *condition_;

				while (!exit_) {
					if (waitProcess()) {
						isWait_ = true;
						condition.wait(uniqueLock, [this, &restartProcess]() { return restartProcess() or exit_; });
					}
					if (exit_) {
						break;
					}
					isWait_ = false;

					userProcess();
				}

				// COM 終了
				CoUninitialize();
				AddLog("CoUninitialize succeeded");
			}
		);

		thraed_ = std::make_unique<std::thread>(*threadProcess_);
	}
}