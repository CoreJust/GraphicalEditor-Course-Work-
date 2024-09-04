#pragma once
#include "State.h"
#include "Expr.h"
#include "../Token.h"

// Statement for built-in instructions
class BuiltInState final : public State {
private:
	std::vector<std::unique_ptr<Expr>> m_args;
	TokenType m_op;

public:
	BuiltInState(TokenType op, std::vector<std::unique_ptr<Expr>> args);

	void generate() override;
};