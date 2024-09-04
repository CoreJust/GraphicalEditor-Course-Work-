#include "ForState.h"
#include "../ByteCodeBuilder.h"

ForState::ForState(
	std::string iterName,
	std::unique_ptr<Expr> rangeFromExpr,
	std::unique_ptr<Expr> rangeToExpr,
	std::unique_ptr<Expr> rangeStepExpr,
	std::unique_ptr<State> body
) :
	m_iterName(std::move(iterName)),
	m_rangeFromExpr(std::move(rangeFromExpr)),
	m_rangeToExpr(std::move(rangeToExpr)),
	m_rangeStepExpr(std::move(rangeStepExpr)),
	m_body(std::move(body))
{
	if (!m_rangeToExpr || m_rangeToExpr->getType() != BasicType::NUMBER
		|| (m_rangeFromExpr && (m_rangeFromExpr->getType() != BasicType::NUMBER
		|| (m_rangeStepExpr && m_rangeStepExpr->getType() != BasicType::NUMBER)))) {
		throw QString("Incorrect range: must have at least one value and all the values must be numbers");
	}
}

void ForState::generate() {
	size_t onConditionFalseLabel = g_builder.getLabel();
	g_builder.addScope(false);

	// Initialization
	Variable stepRangeVar = g_builder.addVariable("$range_step", Type({ BasicType::NUMBER }), false);
	Variable toRangeVar = g_builder.addVariable("$range_to", Type({ BasicType::NUMBER }), false);
	Variable iterVar = g_builder.addVariable(m_iterName, Type({ BasicType::NUMBER }), false);

	stepRangeVar.index = g_builder.getCurrentLocalVarLoc(stepRangeVar.index);
	toRangeVar.index = g_builder.getCurrentLocalVarLoc(toRangeVar.index);
	iterVar.index = g_builder.getCurrentLocalVarLoc(iterVar.index);


	if (!m_rangeFromExpr) {
		g_builder.addInst(Instruction(InstructionType::PUSH, 0.f));
	} else {
		m_rangeFromExpr->generate();
	}

	m_rangeToExpr->generate();

	if (!m_rangeStepExpr) {
		g_builder.addInst(Instruction(InstructionType::PUSH, 1.f));
	} else {
		m_rangeStepExpr->generate();
	}

	g_builder.addInst(Instruction(InstructionType::INIT_RANGE));
	g_builder.addInst(Instruction(InstructionType::ADD_LOCAL));
	g_builder.addInst(Instruction(InstructionType::ADD_LOCAL));
	g_builder.addInst(Instruction(InstructionType::ADD_LOCAL));
	
	// Internal cycle variables must not be cleared on each iteration
	Cycle* cycle = g_builder.addCycle();

	size_t conditionLabel = g_builder.getLabel();
	g_builder.addInst(Instruction(InstructionType::GOTO, uint32_t(conditionLabel)));

	// Increment
	g_builder.setLabelAtNextInst(cycle->beginLabel);
	g_builder.addInst(Instruction(InstructionType::LOAD, uint32_t(iterVar.index)));
	g_builder.addInst(Instruction(InstructionType::LOAD, uint32_t(stepRangeVar.index)));
	g_builder.addInst(Instruction(InstructionType::ADD));
	g_builder.addInst(Instruction(InstructionType::STORE, uint32_t(iterVar.index)));

	// Condition
	g_builder.setLabelAtNextInst(conditionLabel);
	g_builder.addInst(Instruction(InstructionType::LOAD, uint32_t(iterVar.index)));
	g_builder.addInst(Instruction(InstructionType::LOAD, uint32_t(toRangeVar.index)));
	g_builder.addInst(Instruction(InstructionType::LOAD, uint32_t(stepRangeVar.index)));
	g_builder.addInst(Instruction(InstructionType::CHECK_RANGE));
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

	g_builder.addClearScopeInst(3);
	g_builder.deleteScope(false);
	g_builder.endCycle();
}
