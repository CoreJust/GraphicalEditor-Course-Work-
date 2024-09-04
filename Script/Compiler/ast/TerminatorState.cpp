#include "TerminatorState.h"
#include "../ByteCodeBuilder.h"
#include "ValueExpr.h"

TerminatorState::TerminatorState(TerminatorAdded::TerminatorType terminatorType, std::unique_ptr<Expr> arg)
	: m_terminatorType(terminatorType), m_arg(std::move(arg)) {
	switch (m_terminatorType)	{
	case TerminatorAdded::RETURN: break;
	case TerminatorAdded::HALT:
		if (m_arg) {
			throw QString("Halt does not take an argument");
		} break;
	case TerminatorAdded::BREAK:
	case TerminatorAdded::CONTINUE:
		if (m_arg && !m_arg->isConstNumber()) {
			throw QString("Break/continue can only take a constant number or take no arguments at all");
		} break;
	default:
		throw QString("Unknown terminator");
	}
}

void TerminatorState::generate() {
	switch (m_terminatorType) {
	case TerminatorAdded::RETURN:
		m_arg->generate();
		g_builder.addRetInst();
		throw TerminatorAdded{ TerminatorAdded::RETURN };
	case TerminatorAdded::HALT:
		g_builder.addInst(Instruction(InstructionType::HALT));
		throw TerminatorAdded{ TerminatorAdded::HALT };
	case TerminatorAdded::BREAK:
		g_builder.addBreakInst(m_arg ? ((ValueExpr*)m_arg.get())->getConstNumber() : 0);
		throw TerminatorAdded{ TerminatorAdded::BREAK };
	case TerminatorAdded::CONTINUE:
		g_builder.addContinueInst(m_arg ? ((ValueExpr*)m_arg.get())->getConstNumber() : 0);
		throw TerminatorAdded{ TerminatorAdded::CONTINUE };
	default:
		throw QString("Unknown terminator");
	}
}
