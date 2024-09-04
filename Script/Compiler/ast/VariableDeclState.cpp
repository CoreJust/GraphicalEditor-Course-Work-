#include "VariableDeclState.h"

VariableDeclState::VariableDeclState(std::string name, Type type, bool isGlobal, std::unique_ptr<Expr> expr)
	: m_initExpr(std::move(expr)) {
	m_variable = g_builder.addVariable(std::move(name), std::move(type), isGlobal);

	if (m_initExpr->getType() != m_variable.type) {
		throw QString("Variable's and initial expression's types mismatched");
	}
}

void VariableDeclState::generate() {
	m_initExpr->generate();

	g_builder.addVariable(m_variable.name, m_variable.type, m_variable.isGlobal);

	if (m_variable.isGlobal) {
		g_builder.addInst(Instruction(getAccordingToType(InstructionType::STORE_GLOBAL, uint8_t(m_variable.type[0])), m_variable.index));
	} else { // Local variable
		g_builder.addInst(Instruction(getAccordingToType(InstructionType::ADD_LOCAL, uint8_t(m_variable.type[0]))));
	}
}
