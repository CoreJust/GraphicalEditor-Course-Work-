#pragma once
#include "Expr.h"
#include "../Token.h"

// Unary expression (an operator with a single operand)
class UnaryExpr final : public Expr {
private:
	std::unique_ptr<Expr> m_expr;
	TokenType m_op;

public:
	UnaryExpr(TokenType op, std::unique_ptr<Expr> expr);

	void generate() override;

	bool isLVal() override;
};