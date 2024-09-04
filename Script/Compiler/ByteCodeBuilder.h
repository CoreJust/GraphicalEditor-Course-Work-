#pragma once
#include <QList>
#include "../Instruction.h"
#include "ast/Type.h"

struct Variable {
	std::string name;
	Type type;
	uint32_t index; // Its index in variable pool
	bool isGlobal;
	bool scopeFlag = false; // Flag meaning it is not a variable but a marker of a scope
};

struct Function {
	std::string name;
	Type arguments;
	Type returnType;
	size_t position;
};

struct Cycle {
	size_t beginLabel;
	size_t endLabel;

	size_t clearIndex; // Index of the scope variable till which variable stack would be cleared on break/continue
};

// Allows to build a bytecode from AST
// Must be called from AST, does nothing on itself
class ByteCodeBuilder final {
private:
	QList<Instruction> m_byteCode;
	QList<Variable> m_vars;
	QList<Function> m_funcs;
	QList<size_t> m_labels;
	QList<Cycle> m_cycles;
	QList<size_t> m_retClearIndices;

	uint32_t m_globalTop = 0;
	uint32_t m_localTop = 0;

	uint8_t m_imageBuffersCount = 1;

	uint32_t m_maxStackSize = 256 * 256;
	uint32_t m_maxCallStackSize = 2048;
	uint32_t m_maxLocalVariableStackSize = 256 * 256;

public:
	void buildAndStore(QString fileName);

	// Adds a new instruction and returns its index
	size_t addInst(Instruction inst);

	void addBreakInst(uint8_t cycle);
	void addContinueInst(uint8_t cycle);

	// Adds an instruction that destructs -totalCleat- local variables
	void addClearScopeInst(uint32_t totalClear);

	// Adds a RET instruction and clears the stack
	void addRetInst();

	Cycle* addCycle();
	void endCycle();

	void addRetClearIndex();
	void popRetClearIndex();

	// Creates a new label and returns its index
	size_t getLabel();

	// Sets the label's value to the current instruction
	void setLabelAtNextInst(size_t labelId);

	// Replaces all the labels with actual positions
	// Deletes all the labels, thus can only be called once
	void replaceLabels();

	Variable* findVariable(const std::string& name);

	Function* findFunction(const std::string& name, const Type& arguments);
	Function* findFunction(size_t funcId);

	Variable addVariable(std::string name, Type type, bool isGlobal);

	// Returns the functions ID that must be later used for setting up the function to the code
	size_t addFunction(std::string name, Type retType, Type argTypes);

	Function& setFunctionStartLoc(size_t funcId);

	uint32_t getCurrentLocalVarLoc(uint32_t index);

	// modifyInsts is whether the scope added during the codegen and thus auto-clean instructions should be added
	void addScope(bool modifyInsts);
	void deleteScope(bool modifyInsts);

	void setMaxStackSize(uint32_t size);
	void setMaxCallStackSize(uint32_t size);
	void setMaxLocalVariablesStackSize(uint32_t size);

	void setImageBuffersCount(uint8_t count);
	uint8_t getImageBuffersCount();
};

extern ByteCodeBuilder g_builder;