#pragma once
#include "Result.h"

namespace utils {
	// Buffer is a type used to contain raw buffers (dynamic arrays)
	// Non-copiable
	template<typename T>
	class Buffer {
	public:
		// Byte-size of a single element
		static constexpr size_t ElemSize = sizeof(T);

	protected:
		T* m_data = nullptr;

	public:
		Buffer() = default;
		Buffer(const Buffer&) = delete;
		Buffer(Buffer&& other) noexcept
			: m_data(other.m_data) {
			other.m_data = nullptr;
		}

		void operator=(const Buffer&) = delete;
		void operator=(Buffer&& other) noexcept {
			m_data = other.m_data;
			other.m_data = nullptr;
		}

		// Instead of constructors static functions are used

		// Returns a buffer with -size- zero-initialized -T-s or a Failure
		static Result<Buffer<T>> alloc(size_t size) {
			Buffer<T> result;
			if ((result.m_data = (T*)calloc(size, ElemSize)) == nullptr) {
				return Failure("Failed to allocate memory: " + QString::number(size * ElemSize) + " bytes");
			}

			return std::move(result);
		}

		// Returns a buffer with -size- non-initialized -T-s or a Failure
		static Result<Buffer<T>> allocNonInitialized(size_t size) {
			Buffer<T> result;
			if ((result.m_data = (T*)malloc(size * ElemSize)) == nullptr) {
				return Failure("Failed to allocate memory: " + QString::number(size * ElemSize) + " bytes");
			}

			return std::move(result);
		}

		// Frees the memory
		~Buffer() {
			if (m_data != nullptr) {
				free(m_data);
				m_data = nullptr;
			}
		}

		// Tries to reallocate the contained memory for -size- elements and returns either Success or Failure
		Result<Void> realloc(size_t size) {
			if (T* data = (T*)::realloc(m_data, size * ElemSize); data != nullptr) {
				m_data = data;
				return Success();
			}

			return Failure("Failed to reallocate memory: " + QString::number(size * ElemSize) + " bytes");
		}

		bool isEmpty() const noexcept {
			return m_data == nullptr;
		}

		// Returns the contained pointer if there is, or returns Failure if no pointer contained
		T* data() {
			assert(m_data != nullptr && "No data stored: cannot return");

			return m_data;
		}

		// Returns the contained pointer as constant if there is, or returns Failure if no pointer contained
		const T* data() const {
			assert(m_data != nullptr && "No data stored: cannot return");

			return m_data;
		}
	};

	// A Buffer with a size field
	template<typename T>
	class SizedBuffer : public Buffer<T> {
		using Buffer<T>::m_data; // Needed because C++ cannot see base class fields in case of templates

	public:
		// Byte-size of a single element
		static constexpr size_t ElemSize = Buffer<T>::ElemSize;

	protected:
		size_t m_size = 0;

	public:
		SizedBuffer() = default;
		SizedBuffer(SizedBuffer&& other) noexcept
			: Buffer<T>(static_cast<Buffer<T>&&>(other)) {
			m_size = other.m_size;
			other.m_size = 0;
		}

		void operator=(SizedBuffer&& other) noexcept {
			Buffer<T>::operator=(static_cast<Buffer<T>&&>(other));
			m_size = other.m_size;
			other.m_size = 0;
		}

		// Instead of constructors static functions are used

		// Returns a buffer with -size- zero-initialized -T-s or a Failure
		static Result<SizedBuffer<T>> alloc(size_t size) {
			SizedBuffer<T> result;
			if ((result.m_data = (T*)calloc(size, ElemSize)) == nullptr) {
				return Failure("Failed to allocate memory: " + QString::number(size * ElemSize) + " bytes");
			}

			result.m_size = size;
			return std::move(result);
		}

		// Returns a buffer with -size- non-initialized -T-s or a Failure
		static Result<SizedBuffer<T>> allocNonInitialized(size_t size) {
			SizedBuffer<T> result;
			if ((result.m_data = (T*)malloc(size * ElemSize)) == nullptr) {
				return Failure("Failed to allocate memory: " + QString::number(size * ElemSize) + " bytes");
			}

			result.m_size = size;
			return std::move(result);
		}

		// Tries to reallocate the contained memory for -size- elements and returns either Success or Failure
		Result<Void> realloc(size_t size) {
			if (T* data = (T*)::realloc(this->m_data, size * ElemSize); data != nullptr) {
				m_size = size;
				m_data = data;
				return Success();
			}

			return Failure("Failed to reallocate memory: " + QString::number(size * ElemSize) + " bytes");
		}

		// Safe element access
		T& operator[](size_t i) {
			assert(i < m_size);
			return m_data[i];
		}

		// Safe const element access
		const T& operator[](size_t i) const {
			assert(i < m_size);
			return m_data[i];
		}

		// Quick element access, but without index check
		// Must not be used unless absolutely necessary
		T& at(size_t i) {
			return m_data[i];
		}

		// Quick const element access, but without index check
		// Must not be used unless absolutely necessary
		const T& at(size_t i) const {
			return m_data[i];
		}

		// Returns the current number of elements contained
		size_t size() const noexcept {
			return m_size;
		}
	};

	// A sized buffer for a dynamic array of fixed-size dynamic arrays
	template<typename T>
	class SizedBuffer2D final : public SizedBuffer<T*> {
		using Buffer<T*>::m_data; // Needed because C++ cannot see base class fields in case of templates
		using SizedBuffer<T*>::m_size; // Needed because C++ cannot see base class fields in case of templates

	public:
		// Byte-size of a single element
		static constexpr size_t ElemSize = sizeof(T);

	private:
		size_t m_subarraySize = 0; // The size of the allocated arrays

	public:
		SizedBuffer2D() = default;
		SizedBuffer2D(SizedBuffer2D&& other) noexcept
			: SizedBuffer<T*>(static_cast<SizedBuffer<T*>&&>(other)) {
			m_subarraySize = other.m_subarraySize;
			other.m_subarraySize = 0;
		}

		void operator=(SizedBuffer2D&& other) noexcept {
			SizedBuffer<T*>::operator=(static_cast<SizedBuffer<T*>&&>(other));
			m_subarraySize = other.m_subarraySize;
			other.m_subarraySize = 0;
		}

		~SizedBuffer2D() {
			if (m_data != nullptr) {
				for (size_t i = 0; i < m_size; i++) {
					if (m_data[i] != nullptr) {
						free(m_data[i]);
						m_data[i] = nullptr;
					}
				}

				m_data = nullptr;
			}
		}

		// Instead of constructors static functions are used
		static Result<SizedBuffer<T*>> alloc(size_t size) = delete;
		static Result<SizedBuffer<T*>> allocNonInitialized(size_t size) = delete;

		// Returns a buffer with -size1- zero-initialized arrays the size of -size2- or a Failure
		static Result<SizedBuffer2D<T>> alloc(size_t size1, size_t size2) {
			SizedBuffer2D<T> result;
			if ((result.m_data = (T**)malloc(size1 * sizeof(T*))) == nullptr) {
				return Failure("Failed to allocate memory: " + QString::number(size1 * (size2 + 1) * ElemSize) + " bytes");
			}

			result.m_size = size1;
			result.m_subarraySize = size2;
			for (size_t i = 0; i < size1; i++) {
				if ((result.at(i) = (T*)calloc(size2, ElemSize)) == nullptr) {
					return Failure("Failed to allocate memory: " + QString::number(size1 * (size2 + 1) * ElemSize) + " bytes");
				}
			}

			return std::move(result);
		}

		// Returns a buffer with -size- non-initialized arrays the size of -size2- or a Failure
		static Result<SizedBuffer2D<T>> allocNonInitialized(size_t size1, size_t size2) {
			SizedBuffer2D<T> result;
			if ((result.m_data = (T**)malloc(size1 * sizeof(T*))) == nullptr) {
				return Failure("Failed to allocate memory: " + QString::number(size1 * (size2 + 1) * ElemSize) + " bytes");
			}

			result.m_size = size1;
			result.m_subarraySize = size2;
			for (size_t i = 0; i < size1; i++) {
				if ((result.at(i) = (T*)malloc(size2 * ElemSize)) == nullptr) {
					return Failure("Failed to allocate memory: " + QString::number(size1 * (size2 + 1) * ElemSize) + " bytes");
				}
			}

			return std::move(result);
		}

		// No realloc for this buffer
		Result<Void> realloc(size_t size) = delete;

		// Safe element access
		T*& operator[](size_t i) {
			assert(i < m_size);
			return m_data[i];
		}

		// Safe const element access
		const T*& operator[](size_t i) const {
			assert(i < m_size);
			return m_data[i];
		}

		// Quick element access, but without index check
		// Must not be used unless absolutely necessary
		T*& at(size_t i) {
			return m_data[i];
		}

		// Quick const element access, but without index check
		// Must not be used unless absolutely necessary
		const T*& at(size_t i) const {
			return m_data[i];
		}
	};
}