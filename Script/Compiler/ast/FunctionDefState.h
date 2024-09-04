#pragma once
#include "State.h"
#include "../ByteCodeBuilder.h"

// Statement for functions's declaration
class FunctionDefState final : public State {
private:
	size_t m_function;
	QList<Variable> m_args;
	std::unique_ptr<State> m_body;

public:
	FunctionDefState(std::string name, Type retType, QList<Variable> args, Type argTypes, std::unique_ptr<State> body);

	void generate() override;

	bool isFunction() override;
};