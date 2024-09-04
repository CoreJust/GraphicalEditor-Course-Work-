#pragma once
#include <QString>
#include "Void.h"

namespace utils {
	// An intermediate type to create a failure Result, not for a separate use
	// Use it like Failure(message) to initialize a failed result
	// Non-copiable and non-movable
	struct Failure {
		QString message;

		Failure(QString msg = "");

		// Prohibited constructors/operators
		Failure(Failure&&) = delete;
		Failure(const Failure&) = delete;

		Failure& operator=(Failure&&) = delete;
		Failure& operator=(const Failure&) = delete;
	};


	// Result is a class used for function's return value, similar to c++23's std::expected
	// It contains a boolean value whether the function executed successfully or not
	// In case of success, the Result holds the return value
	// In case of a failure, the Result holds an error message
	// Convenient for error handling and better (IMHO) than exceptions
	// Non-copiable, but movable
	// 
	// Instead of Result<void> Result<Void> must be used
	// 
	// Usage:	return Failure(-message-) or Result(Failure(-message-)) for error
	//			return Result(-value-) on success
	//			return Success() or Result(Success()) - specifically for Result<Void>, since return Result<Void>() would look bad
	//
	// On getting a Result from a function it must first be verified:
	//			auto result = some_func_that_returns_Result();
	//			if (!result.isOk()) - handle the error, within the if result.error() can be called
	//			... -  result.value() or result.extract() can be called
	template<typename Value_type>
	class Result final {
		static_assert(!std::is_same<Value_type, Failure>::value, "Cannot contain the Failure type");

	private:
		bool m_isOk; // Is the result successful and thus containing a value
		bool m_isNonEmpty = true; // Flag, set to 0 only if the Result was memset-ed to 0 or the value was extracted
		bool m_isVerified = false; // Flag, set to false by default and changes to true if isOk() is called
		union {
			Value_type m_value;
			QString m_errorMessage;
		};

	public:
		Result() = delete;
		Result(Result&) = delete;

		// Moving constructor
		Result(Result&& r) {
			std::memmove(this, &r, sizeof(Result<Value_type>));
			std::memset(&r, 0, sizeof(Result<Value_type>)); // Apart from other things, sets m_isNonEmpty to 0
		}

		Result(Value_type value)
			: m_isOk(true), m_value(std::move(value)) {

		}

		Result(Failure&& failure)
			: m_isOk(false), m_errorMessage(std::move(failure.message)) {

		}

		~Result() {
			if (!m_isNonEmpty) return;
			assert(m_isVerified && "Must be verified before destruction");

			if (m_isOk) {
				m_value.~Value_type();
			}
			else {
				m_errorMessage.~QString();
			}

			m_isNonEmpty = false; // Just in case
		}

		// Is the result success (true) or failure (false)
		inline bool isOk() {
			assert(m_isNonEmpty);

			m_isVerified = true;
			return m_isOk;
		}

		// The error message, can be called only if isOk() is false
		inline const QString& error() const {
			assert(!m_isOk && "Cannot get an error message from a successful result");
			assert(m_isNonEmpty && "Cannot get an error message from an empty result");
			assert(m_isVerified && "Cannot get an error message from an unverified result");

			return m_errorMessage;
		}

#pragma warning(push)
#pragma warning(disable : 4172) // Compiler dislikes return a union member, but it is fine in this case
		// Extracting the error as a Failure, can be called only if isOk() is false
		// Intended to be used when you need to pass a Failure to a Result of another type
		inline Failure extractError() const {
			assert(!m_isOk && "Cannot get an error message from a successful result");
			assert(m_isNonEmpty && "Cannot get an error message from an empty result");
			assert(m_isVerified && "Cannot get an error message from an unverified result");

			return Failure(std::move(m_errorMessage));
		}
#pragma warning(pop)

		// Accessing the value, can be called only if isOk() is true
		inline const Value_type& value() {
			assert(m_isOk && "Cannot get a returned value from an unsuccessful result");
			assert(m_isNonEmpty && "Cannot get a returned value from an empty result");
			assert(m_isVerified && "Cannot get a returned value from an unverified result");

			return m_value;
		}

		// Extracting the value, makes the Result empty, can be called only if isOk() is true
		inline Value_type&& extract() {
			assert(m_isOk && "Cannot extract returned value from an unsuccessful result");
			assert(m_isNonEmpty && "Cannot extract returned value from an empty result");
			assert(m_isVerified && "Cannot extract returned value from an unverified result");

			m_isNonEmpty = false;
			return std::move(m_value);
		}
	};

	// For Result<Void>, since you wouldn't do something like return Result(Result<void>::Value_type())
	// Used instead of a value for successful Result<void>
	using Success = Void;
}