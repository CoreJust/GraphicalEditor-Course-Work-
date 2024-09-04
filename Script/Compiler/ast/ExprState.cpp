#include "ExprState.h"
#include "../ByteCodeBuilder.h"

ExprState::ExprState(std::unique_ptr<Expr> expr)
	: m_expr(std::move(expr)) {

}

void ExprState::generate() {
	m_expr->generate();

	for (auto type : m_expr->getType()) {
		g_builder.addInst(Instruction(getAccordingToType(InstructionType::POP, uint8_t(type))));
	}
}
