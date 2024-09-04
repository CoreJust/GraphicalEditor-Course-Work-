#include "FunctionCallExpr.h"

FunctionCallExpr::FunctionCallExpr(std::string name, std::vector<std::unique_ptr<Expr>> arguments)
	: m_args(std::move(arguments)) {
	Type argTypes;
	for (auto& arg : m_args) {
		for (BasicType type : arg->getType()) {
			argTypes.push_back(type);
		}
	}

	Function* func = g_builder.findFunction(name, argTypes);
	if (func == nullptr) {
		throw QString("No function found with name " + QString::fromStdString(name) 
			+ " and with number of arguments: " + QString::number(argTypes.size()));
	}

	m_type = func->returnType;
	m_function = func->position; // Function ID here
}

void FunctionCallExpr::generate() {
	Function* func = g_builder.findFunction(m_function);

	for (auto arg = m_args.rbegin(); arg != m_args.rend(); arg++) {
		(*arg)->generate();
	}

	g_builder.addInst(Instruction(InstructionType::CALL, (uint32_t)func->position));
}

bool FunctionCallExpr::isLVal() {
	return false;
}
