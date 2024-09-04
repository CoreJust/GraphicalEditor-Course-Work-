#include "FunctionDefState.h"

FunctionDefState::FunctionDefState(
	std::string name,
	Type retType,
	QList<Variable> args,
	Type argTypes,
	std::unique_ptr<State> body
) :
	m_body(std::move(body)),
	m_args(std::move(args))
{
	m_function = g_builder.addFunction(std::move(name), std::move(retType), std::move(argTypes));
}

void FunctionDefState::generate() {
	size_t skipLabel = g_builder.getLabel();
	g_builder.addInst(Instruction(InstructionType::GOTO, uint32_t(skipLabel)));

	Function& func = g_builder.setFunctionStartLoc(m_function);

	g_builder.addScope(false);
	g_builder.addRetClearIndex();

	for (auto& arg : m_args) {
		BasicType argType = arg.type[0];
		g_builder.addVariable(std::move(arg.name), std::move(arg.type), false);
		g_builder.addInst(Instruction(getAccordingToType(InstructionType::ADD_LOCAL, uint8_t(argType)))); // loading arguments
	}

	try {
		m_body->generate();
		g_builder.addRetInst();
	} catch (TerminatorAdded terminator) {
		if (terminator.type != TerminatorAdded::RETURN && terminator.type != TerminatorAdded::HALT) {
			throw QString("No break/continue can be used outside a cycle");
		}
	}

	g_builder.popRetClearIndex();
	g_builder.deleteScope(false);

	g_builder.setLabelAtNextInst(skipLabel);
}

bool FunctionDefState::isFunction() {
	return true;
}
