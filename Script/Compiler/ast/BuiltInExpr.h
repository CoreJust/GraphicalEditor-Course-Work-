#pragma once
#include "Expr.h"
#include "../Token.h"

// Built-in expression (a constant or built-in function)
class BuiltInExpr final : public Expr {
private:
	std::vector<std::unique_ptr<Expr>> m_args;
	TokenType m_op;

public:
	BuiltInExpr(TokenType op);
	BuiltInExpr(TokenType op, std::vector<std::unique_ptr<Expr>> args);

	void generate() override;

	bool isLVal() override;
};