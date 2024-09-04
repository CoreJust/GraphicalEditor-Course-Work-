#include "ValueExpr.h"
#include "../ByteCodeBuilder.h"

ValueExpr::ValueExpr(float val) 
	: Expr({ BasicType::NUMBER }) {
	m_value[0] = val;
}

ValueExpr::ValueExpr(float x, float y)
	: Expr({ BasicType::POINT }) {
	m_value[0] = x;
	m_value[1] = y;
}

ValueExpr::ValueExpr(float r, float g, float b, float a)
	: Expr({ BasicType::COLOR }) {
	m_value[0] = r;
	m_value[1] = g;
	m_value[2] = b;
	m_value[3] = a;
}

void ValueExpr::generate() {
	switch (m_type[0]) {
		case BasicType::NUMBER:
			g_builder.addInst(Instruction(InstructionType::PUSH, m_value[0]));
			return;
		case BasicType::POINT:
			g_builder.addInst(Instruction(InstructionType::PUSH_POINT, m_value[0], m_value[1]));
			return;
		case BasicType::COLOR:
			g_builder.addInst(Instruction(InstructionType::PUSH_POINT, m_value[0], m_value[1]));
			g_builder.addInst(Instruction(InstructionType::PUSH_POINT, m_value[2], m_value[3]));
			return;
	default:
		break;
	}
}

bool ValueExpr::isLVal() {
	return false;
}

bool ValueExpr::isConstNumber() {
	return m_type == BasicType::NUMBER;
}

float ValueExpr::getConstNumber() {
	assert(m_type == BasicType::NUMBER);

	return m_value[0];
}
