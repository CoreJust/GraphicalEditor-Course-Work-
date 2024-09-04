#pragma once
#include "Expr.h"

// Expression of a single value (number, point, or color)
class ValueExpr final : public Expr {
private:
	float m_value[4] = { 0, 0, 0, 0 };

public:
	ValueExpr(float val);
	ValueExpr(float x, float y);
	ValueExpr(float r, float g, float b, float a);

	void generate() override;

	bool isLVal() override;
	bool isConstNumber() override;

	// Returns the contained number
	float getConstNumber();
};