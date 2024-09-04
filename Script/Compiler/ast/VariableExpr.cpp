#include "VariableExpr.h"
#include "../ByteCodeBuilder.h"

VariableExpr::VariableExpr(std::string name) {
	Variable* var = g_builder.findVariable(name);
	if (var == nullptr) {
		throw QString("No such variable found: " + QString::fromStdString(name));
	}

	if (m_isGlobal = var->isGlobal) {
		m_index = var->index;
	} else {
		m_index = g_builder.getCurrentLocalVarLoc(var->index);
	}

	m_type = var->type;
}

void VariableExpr::generate() {
	if (!m_isGlobal) {
		g_builder.addInst(Instruction(
			getAccordingToType(InstructionType::LOAD, (uint8_t)m_type[0]),
			m_index
		));
	} else {
		g_builder.addInst(Instruction(
			getAccordingToType(InstructionType::LOAD_GLOBAL, (uint8_t)m_type[0]),
			m_index
		));
	}
}

bool VariableExpr::isLVal() {
	return true;
}
