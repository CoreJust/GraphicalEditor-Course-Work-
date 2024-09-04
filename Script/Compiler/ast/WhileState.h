#pragma once
#include "State.h"
#include "Expr.h"

// Statement for the while cycle
class WhileState final : public State {
private:
	std::unique_ptr<State> m_body;
	std::unique_ptr<Expr> m_condition;

public:
	WhileState(std::unique_ptr<State> body, std::unique_ptr<Expr> condition);

	void generate() override;
};