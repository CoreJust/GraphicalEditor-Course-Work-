#pragma once
#include "Type.h"

// Basic class for all expressions
class Expr {
protected:
	Type m_type;

public:
	Expr() = default;

	Expr(Type type);

	// Translates the expression into a set of instructions stored in a global state
	// It is guaranteed that the set of instructions would leave a new value on the stack according to expression's type
	virtual void generate() = 0;

	// If it is an LValue
	virtual bool isLVal() = 0;

	// Is it a compile-time number
	virtual bool isConstNumber();

	// The expression's type
	const Type& getType();
};

Type getCommonType(const Type& a, const Type& b);