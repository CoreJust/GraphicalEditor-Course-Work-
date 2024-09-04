#pragma once
#include "Expr.h"

// Expression of getting a variable value
class VariableExpr final : public Expr {
private:
	uint32_t m_index;
	bool m_isGlobal;

public:
	VariableExpr(std::string name);

	void generate() override;

	bool isLVal() override;
};