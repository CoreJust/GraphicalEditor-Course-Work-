#pragma once
#include "State.h"
#include "Expr.h"

// Statement for terminator instructions (flow-control: break, continue, return, halt)
class TerminatorState final : public State {
private:
	std::unique_ptr<Expr> m_arg;
	TerminatorAdded::TerminatorType m_terminatorType;

public:
	TerminatorState(TerminatorAdded::TerminatorType terminatorType, std::unique_ptr<Expr> arg);

	void generate() override;
};