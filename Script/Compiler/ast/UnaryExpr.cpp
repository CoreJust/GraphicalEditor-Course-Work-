#include "UnaryExpr.h"
#include "../ByteCodeBuilder.h"

UnaryExpr::UnaryExpr(TokenType op, std::unique_ptr<Expr> expr)
	: Expr(expr->getType()), m_op(op), m_expr(std::move(expr)) {
	assert(op == TokenType::NOT || op == TokenType::MINUS);
}

void UnaryExpr::generate() {
	m_expr->generate();

	if (m_op == TokenType::NOT) {
		if (m_type.size() != 1 && m_type[0] != BasicType::NUMBER) {
			throw QString("Not is only applicable to numbers!");
		}

		g_builder.addInst(Instruction(InstructionType::NOT));
	} else {
		if (m_type.size() != 1) {
			throw QString("Cannot negate a tuple");
		}

		g_builder.addInst(Instruction(getAccordingToType(InstructionType::NEG, uint8_t(m_type[0]))));
	}
}

bool UnaryExpr::isLVal() {
	return false;
}
