#pragma once
#include "State.h"
#include "Expr.h"

// Statement for the if-else structure
class IfElseState final : public State {
private:
	std::unique_ptr<State> m_body;
	std::unique_ptr<State> m_elseBody;
	std::unique_ptr<Expr> m_condition;

public:
	IfElseState(std::unique_ptr<Expr> condition, std::unique_ptr<State> body, std::unique_ptr<State> elseBody);

	void generate() override;
};