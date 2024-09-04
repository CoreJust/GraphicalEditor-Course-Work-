#pragma once
#include <cstdint>

// Basic class for all the statements
class State {
public:
	// Translates the statement into a set of instructions stored in a global state
	virtual void generate() = 0;

	inline virtual bool isFunction() {
		return false;
	}
};

// Thrown on terminator being added in a block of code
struct TerminatorAdded {
	enum TerminatorType : uint8_t {
		RETURN = 0,
		HALT,

		BREAK,
		CONTINUE
	};

	TerminatorType type;
};