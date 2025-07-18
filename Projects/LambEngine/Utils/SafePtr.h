#pragma once
#include <utility>
#include "Error/Error.h"
#include "SafeDelete.h"
#ifdef USE_DEBUG_CODE
#include <cassert>
#endif // USE_DEBUG_CODE


namespace Lamb {
	/// <summary>
	/// <para>ヌルアクセスを許さないポインタ</para>
	/// <para>自動開放機能はない</para>
	/// </summary>
	/// <typeparam name="T"></typeparam>
	template<IsObject T>
	class SafePtr {
	public:
		using type = T;

	public:
		SafePtr():
			ptr_(nullptr)
		{};
		SafePtr(T* ptr) :
			ptr_(ptr)
		{}
		SafePtr(std::nullptr_t) :
			ptr_(nullptr)
		{}
		SafePtr(const SafePtr&) = default;
		SafePtr(SafePtr&&) = default;
		~SafePtr() = default;

	public:
		SafePtr& operator=(std::nullptr_t) {
			ptr_ = nullptr;

			return *this;
		}
		SafePtr& operator=(T* ptr) {
			ptr_ = ptr;

			return *this;
		}
		SafePtr& operator=(const SafePtr&) = default;
		SafePtr& operator=(SafePtr&& right) noexcept {
			ptr_ = right.release();

			return *this;
		}

	public:
		explicit operator bool() const {
			return !!ptr_;
		}
		bool operator!() const {
			return !ptr_;
		}

		bool operator==(const SafePtr& right) const {
			return ptr_ == right.ptr_;
		}
		bool operator!=(const SafePtr& right) const {
			return ptr_ != right.ptr_;
		}
		bool operator==(T* right) const {
			return ptr_ == right;
		}
		bool operator!=(T* right) const {
			return ptr_ != right;
		}

		T** operator&()  {
			return &ptr_;
		}
		T*const* operator&() const {
			return &ptr_;
		}

		T* operator->()  {
			if (empty()) [[unlikely]] {
#ifdef USE_DEBUG_CODE
				assert(!"nullptr access");
#else
				NullPointerException(ErrorPlace);
#endif // USE_DEBUG_CODE
			}
			return ptr_;
		}
		T* const operator->() const  {
			if (empty()) [[unlikely]] {
#ifdef USE_DEBUG_CODE
				assert(!"nullptr access");
#else
				NullPointerException(ErrorPlace);
#endif // USE_DEBUG_CODE
			}
			return ptr_;
		}

		T& operator*()  {
			if (empty()) [[unlikely]] {
#ifdef USE_DEBUG_CODE
				assert(!"nullptr access");
#else
				NullPointerException(ErrorPlace);
#endif // USE_DEBUG_CODE
			}
			return *ptr_;
		}

		T& operator*() const  {
			if (empty()) [[unlikely]] {
#ifdef USE_DEBUG_CODE
				assert(!"nullptr access");
#else
				NullPointerException(ErrorPlace);
#endif // USE_DEBUG_CODE
			}
			return *ptr_;
		}

		T& operator[](size_t index)  {
			if (empty()) [[unlikely]] {
#ifdef USE_DEBUG_CODE
				assert(!"nullptr access");
#else
				NullPointerException(ErrorPlace);
#endif // USE_DEBUG_CODE
			}
			return (ptr_[index]);
		}
		T& operator[](size_t index) const  {
			if (empty()) [[unlikely]] {
#ifdef USE_DEBUG_CODE
				assert(!"nullptr access");
#else
				NullPointerException(ErrorPlace);
#endif // USE_DEBUG_CODE

			}
			return (ptr_[index]);
		}

		// 後置インクリメント
		SafePtr operator++(int32_t) {
			SafePtr tmp = ptr_++;
			return tmp;
		}
		// 後置デクリメント
		SafePtr operator--(int32_t) {
			SafePtr tmp = ptr_--;
			return tmp;
		}

		// 前置インクリメント
		SafePtr& operator++() {
			++ptr_;
			return *this;
		}
		// 前置デクリメント
		SafePtr& operator--() {
			--ptr_;
			return *this;
		}

		/// <summary>
		/// 通常ポインタを取得
		/// </summary>
		/// <returns>ポインタ</returns>
		T* get() {
			return ptr_;
		}
		/// <summary>
		/// 通常ポインタを取得
		/// </summary>
		/// <returns>ポインタ</returns>
		T* const get() const {
			return ptr_;
		}

		/// <summary>
		/// 通常ポインタを取得し、nullptrを代入する
		/// </summary>
		/// <returns>ポインタ</returns>
		T* release() {
			T* result = ptr_;
			ptr_ = nullptr;
			return result;
		}

		/// <summary>
		/// 解放
		/// </summary>
		void reset() {
			Lamb::SafeDelete(ptr_);
		}
		/// <summary>
		/// 解放して新しいポインタを代入
		/// </summary>
		/// <param name="ptr">ポインタ</param>
		void reset(T* ptr) {
			reset();
			ptr_ = ptr;
		}

		void reset(const Lamb::SafePtr<T>& ptr) {
			reset();
			*this = ptr;
		}

		bool have() const {
			return !!ptr_;
		}

		bool empty() const {
			return !ptr_;
		}

		constexpr size_t byte_size() const {
			return sizeof(T);
		}

		void*const* GetPtrAdress() const {
			return reinterpret_cast<void*const*>(&ptr_);
		}
		void** GetPtrAdress() {
			return reinterpret_cast<void**>(&ptr_);
		}

	public:
		/// <summary>
		/// ヌルの場合は例外をthrowする
		/// </summary>
		/// <param name="funcName">関数名</param>
		template<class Name = SafePtr<T>>
		inline void NullCheck(
			const std::string& funcName,
			const std::string& sourceFileName,
			uint32_t codeLineNumber
		) const {
			if (empty()) {
				NullPointerException<Name>(funcName, sourceFileName, codeLineNumber);
			}
		}

	private:
		template<class Name = SafePtr<T>>
		void NullPointerException(
			const std::string& funcName,
			const std::string& sourceFileName,
			uint32_t codeLineNumber
		) const {
			throw Lamb::Error::Code<Name>("NullPointerException", funcName, sourceFileName, codeLineNumber);
		}

	private:
		T* ptr_;
	};

	template<class T, class... Types>
	SafePtr<T> MakeSafePtr(Types&&... args) {
		return SafePtr<T>(new T(std::forward<Types>(args)...));
	}
}

template<typename T, typename U>
bool operator==(const Lamb::SafePtr<T>& left, const Lamb::SafePtr<U>& right) {
	return (void*)left.get() == (void*)right.get();
}

namespace std {
	template<typename T>
	struct hash<Lamb::SafePtr<T>> {
		size_t operator()(const Lamb::SafePtr<T>& key) const {
			return std::hash<const T*>()(key.get());
		}
	};
}


