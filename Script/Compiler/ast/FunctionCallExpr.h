#pragma once
#include "Expr.h"
#include "../ByteCodeBuilder.h"

// Binary expression (an operator with 2 operands)
class FunctionCallExpr final : public Expr {
private:
	size_t m_function;
	std::vector<std::unique_ptr<Expr>> m_args;

public:
	FunctionCallExpr(std::string name, std::vector<std::unique_ptr<Expr>> arguments);

	void generate() override;

	bool isLVal() override;
};