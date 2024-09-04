#include "IfElseState.h"
#include "../ByteCodeBuilder.h"

IfElseState::IfElseState(std::unique_ptr<Expr> condition, std::unique_ptr<State> body, std::unique_ptr<State> elseBody)
	: m_condition(std::move(condition)), m_body(std::move(body)), m_elseBody(std::move(elseBody)) {
	if (m_condition->getType() != BasicType::NUMBER) {
		throw QString("Only a number can be a condition");
	}
}

void IfElseState::generate() {
	size_t onConditionFalseLabel = g_builder.getLabel();
	size_t onIfBodyEndLabel = g_builder.getLabel();
	g_builder.addScope(false);

	// Condition
	m_condition->generate();
	g_builder.addInst(Instruction(InstructionType::GOTO_IF_NOT, uint32_t(onConditionFalseLabel)));

	// If-body
	try {
		m_body->generate();
		if (m_elseBody) {
			g_builder.addInst(Instruction(InstructionType::GOTO, uint32_t(onIfBodyEndLabel)));
		}
	} catch (TerminatorAdded terminator) {
		// Nothing to do here
	}

	// Else-body
	try {
		g_builder.setLabelAtNextInst(onConditionFalseLabel);
		if (m_elseBody) {
			g_builder.deleteScope(false);
			g_builder.addScope(false);

			m_elseBody->generate();
			g_builder.setLabelAtNextInst(onIfBodyEndLabel);
		}
	} catch (TerminatorAdded terminator) {
		// Nothing to do here
	}

	g_builder.deleteScope(false);
}
