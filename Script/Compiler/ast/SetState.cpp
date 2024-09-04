#include "SetState.h"

SetState::SetState(std::string name, Field field, std::unique_ptr<Expr> expr)
	: m_expr(std::move(expr)), m_field(field) {
	Variable* var = g_builder.findVariable(name);
	if (var == nullptr) {
		throw QString("No such variable found: " + QString::fromStdString(name));
	}

	m_variable = *var;

	BasicType varType = m_variable.type[0];
	if (field == Field::RED || field == Field::GREEN || field == Field::BLUE || field == Field::ALPHA) {
		if (varType != BasicType::COLOR) {
			throw QString("Only colors can have rgba fields");
		}

		varType = BasicType::NUMBER;
	} else if (field == Field::X || field == Field::Y) {
		if (varType != BasicType::POINT) {
			throw QString("Only points can have xy fields");
		}

		varType = BasicType::NUMBER;
	}
	
	if (m_expr->getType() != varType) {
		throw QString("Cannot set: incompatible types");
	}

	m_fieldType = varType;

	if (!m_variable.isGlobal) {
		m_variable.index = g_builder.getCurrentLocalVarLoc(m_variable.index);
	}
}

void SetState::generate() {
	m_expr->generate();

	uint8_t fieldPos = 0;

	switch (m_field) {
		case SetState::RED: fieldPos = 0; break;
		case SetState::GREEN: fieldPos = 1; break;
		case SetState::BLUE: fieldPos = 2; break;
		case SetState::ALPHA: fieldPos = 3; break;
		case SetState::X: fieldPos = 0; break;
		case SetState::Y: fieldPos = 1; break;
	default: break;
	}

	if (m_variable.isGlobal) {
		g_builder.addInst(Instruction(getAccordingToType(InstructionType::STORE_GLOBAL, uint8_t(m_fieldType)), m_variable.index + fieldPos));
	} else {
		g_builder.addInst(Instruction(getAccordingToType(InstructionType::STORE, uint8_t(m_fieldType)), m_variable.index + fieldPos));
	}
}
