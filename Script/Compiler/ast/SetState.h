#pragma once
#include "State.h"
#include "Expr.h"
#include "../ByteCodeBuilder.h"

// Statement for the 'set' instruction (assignment)
class SetState final : public State {
public:
	enum Field : uint8_t {
		NONE = 0,

		RED,
		GREEN,
		BLUE,
		ALPHA,

		X,
		Y
	};

private:
	Variable m_variable;
	std::unique_ptr<Expr> m_expr;
	Field m_field = NONE;
	BasicType m_fieldType;

public:
	SetState(std::string name, Field field, std::unique_ptr<Expr> expr);

	void generate() override;
};