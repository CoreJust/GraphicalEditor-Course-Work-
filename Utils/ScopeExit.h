#pragma once

// A utility that allows to execute some code on exiting the scope
// Example:
//		at_scope_exit { std::cout << "Left the scope!"; };
//		std::cout << "Some output\n";
//		return;
//
// The result would be:
//		Some output
//		Left the scope!
//
// Note that it wouldn't work with flow-control: returns, breaks, continues, etc

struct _ScopeExitNote {};

template<class Func>
struct _ScopeExitImpl {
	Func f;

	~_ScopeExitImpl() {
		f();
	}
};

template<class Func>
_ScopeExitImpl<Func> operator^(_ScopeExitNote, Func f) {
	return _ScopeExitImpl(f);
}

// Generating a name for
#define GEN_NAME_(id, LINE) id ## LINE
#define GEN_ASE_NAME(LINE) GEN_NAME_(ASE__, LINE)

// Allows to replace something like:
//		at_scope_exit { fclose(file); };
// with something like:
//		auto ASE__135 = _ScopeExitNote{} ^ [&]() { fclose(file); };
// which creates an instance of _ScopeExitImpl that on destruction calls the lambda
#define at_scope_exit auto GEN_ASE_NAME(__LINE__) = _ScopeExitNote{} ^ [&]()