#include "WhileState.h"
#include "../ByteCodeBuilder.h"

WhileState::WhileState(std::unique_ptr<State> body, std::unique_ptr<Expr> condition)
	: m_body(std::move(body)), m_condition(std::move(condition)) {
	if (m_condition->getType() != BasicType::NUMBER) {
		throw QString("Only a number can be a condition");
	}
}

void WhileState::generate() {
	Cycle* cycle = g_builder.addCycle();
	size_t onConditionFalseLabel = g_builder.getLabel();
	g_builder.addScope(false);
	g_builder.setLabelAtNextInst(cycle->beginLabel);

	// Condition
	m_condition->generate();
	g_builder.addInst(Instruction(InstructionType::GOTO_IF_NOT, uint32_t(onConditionFalseLabel)));

	// Body
	try {
		m_body->generate();
		g_builder.addContinueInst(0);
	} catch (TerminatorAdded terminator) {
		// Nothing to do here
	}
	
	// Cycle end
	g_builder.setLabelAtNextInst(onConditionFalseLabel);
	g_builder.addBreakInst(0);
	g_builder.setLabelAtNextInst(cycle->endLabel);

	g_builder.deleteScope(false);
	g_builder.endCycle();
}
