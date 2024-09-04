#pragma once
#include "Expr.h"
#include "../Token.h"

// Binary expression (an operator with 2 operands)
class BinaryExpr final : public Expr {
private:
	std::unique_ptr<Expr> m_left;
	std::unique_ptr<Expr> m_right;
	TokenType m_op;

public:
	BinaryExpr(TokenType op, std::unique_ptr<Expr> left, std::unique_ptr<Expr> right);

	void generate() override;

	bool isLVal() override;
};