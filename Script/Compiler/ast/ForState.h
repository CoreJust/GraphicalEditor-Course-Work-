#pragma once
#include "State.h"
#include "Expr.h"

// Statement for the for cycle
class ForState final : public State {
private:
	std::string m_iterName;
	std::unique_ptr<Expr> m_rangeFromExpr;
	std::unique_ptr<Expr> m_rangeToExpr;
	std::unique_ptr<Expr> m_rangeStepExpr;
	std::unique_ptr<State> m_body;

public:
	ForState(
		std::string iterName,
		std::unique_ptr<Expr> rangeFromExpr,
		std::unique_ptr<Expr> rangeToExpr,
		std::unique_ptr<Expr> rangeStepExpr,
		std::unique_ptr<State> body
	);

	void generate() override;
};