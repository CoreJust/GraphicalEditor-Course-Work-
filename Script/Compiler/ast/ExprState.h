#pragma once
#include "State.h"
#include "Expr.h"

// Statement for expression used where a state was supposed to be
// Executes the expression and clears the stack after it
class ExprState final : public State {
private:
	std::unique_ptr<Expr> m_expr;

public:
	ExprState(std::unique_ptr<Expr> expr);

	void generate() override;
};