#pragma once
#include "Expr.h"

// Allows to get access to a field
class FieldAccessExpr final : public Expr {
public:
	enum Field : uint8_t {
		RED = 1, // Just in case, to be compatible with SetState's Field
		GREEN,
		BLUE,
		ALPHA,

		X,
		Y
	};

private:
	std::unique_ptr<Expr> m_expr;
	Field m_field;

public:
	FieldAccessExpr(Field field, std::unique_ptr<Expr> expr);

	void generate() override;

	bool isLVal() override;
};