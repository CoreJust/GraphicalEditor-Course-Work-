#pragma once
#include "State.h"
#include "Expr.h"
#include "../ByteCodeBuilder.h"

// Statement for variable's declaration
class VariableDeclState final : public State {
private:
	Variable m_variable;
	std::unique_ptr<Expr> m_initExpr;

public:
	VariableDeclState(std::string name, Type type, bool isGlobal, std::unique_ptr<Expr> expr);

	void generate() override;
};