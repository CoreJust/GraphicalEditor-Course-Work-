#pragma once
#include <cstdint>

// Type of bytecode's instruction
enum class InstructionType : uint8_t {
	NOPE = 0,

	// Stack
	PUSH, // Pushes a single number to the stack
	PUSH_POINT, // Pushes 2 numbers to the stack

	POP, // Pops a single number from the stack
	POP_POINT, // Pops a point (2 numbers) from the stack
	POP_COLOR, // Pops a color (4 numbers) from the stack

	// Used when there is a color/point value on the stack and one of the fields is required
	// Leaves only the required field's number value on the stack
	GET_COLOR_RED,
	GET_COLOR_GREEN,
	GET_COLOR_BLUE,
	GET_COLOR_ALPHA,
	GET_POINT_X,
	GET_POINT_Y,

	// Variables
	// Creating local variables
	ADD_LOCAL,
	ADD_LOCAL_POINT,
	ADD_LOCAL_COLOR,

	CLEAR_SCOPE, // Delete scope's local variables

	// Loading a local variable to the stack
	LOAD,
	LOAD_POINT,
	LOAD_COLOR,

	// Storing a local variable from the stack
	STORE,
	STORE_POINT,
	STORE_COLOR,

	// Loading a global variable to the stack
	LOAD_GLOBAL,
	LOAD_POINT_GLOBAL,
	LOAD_COLOR_GLOBAL,

	// Storing a global variable from the stack
	STORE_GLOBAL,
	STORE_POINT_GLOBAL,
	STORE_COLOR_GLOBAL,

	// Arithmetics
	ADD,
	ADD_POINT, // Sum of points
	ADD_COLOR, // Sum of colors

	ADD_TO_POINT, // Sum of a number and a point
	ADD_TO_COLOR, // Sum of a number and a color

	SUB,
	SUB_POINT, // Difference of points
	SUB_COLOR, // Difference of colors

	SUB_FROM_POINT, // Substraction of a number from a point
	SUB_FROM_COLOR, // Substraction of a number from a color

	MUL,
	MUL_POINT_ON_NUMBER,
	MUL_COLOR_ON_NUMBER,

	DIV,
	DIV_POINT_ON_NUMBER,
	DIV_COLOR_ON_NUMBER,

	MOD,
	MOD_POINT_ON_NUMBER,
	MOD_COLOR_ON_NUMBER,

	POW,
	POW_POINT_TO_NUMBER,
	POW_COLOR_TO_NUMBER,

	NEG,
	NEG_POINT,
	NEG_COLOR,

	INC, // Increments the number at the top
	DEC, // Decrements the number at the top

	// Logics
	NOT,

	CMP_EQ,
	CMP_EQ_POINTS,
	CMP_EQ_COLORS,

	CMP_NEQ,
	CMP_NEQ_POINTS,
	CMP_NEQ_COLORS,

	// For numbers only
	CMP_LT,
	CMP_GT,
	CMP_GE,
	CMP_LE,

	AND,
	OR,

	// Jumps
	GOTO,
	GOTO_IF_NOT,

	CALL,
	RET,

	HALT,

	// Built-ins
	INIT_RANGE, // sorts three values on the stack to be used as (from, to, step)
	CHECK_RANGE, // checks whether the iter-variable is out of range (and thus the cycle is finished)

	PUSH_WIDTH,
	PUSH_HEIGHT,

	SET_IMAGE,
	COPY_IMAGE,
	UPDATE,

	SET_COLOR,
	SET_WIDTH,

	DRAW_PIX,
	DRAW_STROKE,
	DRAW_LINE,
	DRAW_RECT,
	DRAW_CIRCLE,

	FILL_RECT,
	FILL_CIRCLE,

	SLEEP,

	ABS,
	MIN,
	MAX,
	SUM,

	ROUND,
	FLOOR,
	CEIL,

	SIN,
	COS,
	TAN,
	COT,

	EXP,
	LOG,

	LENGTH,
	DISTANCE
};

uint8_t getInstructionArgsSize(InstructionType type);

inline InstructionType getAccordingToType(InstructionType inst, uint8_t type) {
	return (InstructionType)(uint8_t(inst) + type);
}

struct Instruction {
	InstructionType op;

	uint32_t value[2];

	inline Instruction(InstructionType type)
		: op(type) {
		value[0] = 0;
		value[1] = 0;
	}

	inline Instruction(InstructionType type, uint32_t val)
		: op(type) {
		value[0] = val;
		value[1] = 0;
	}

	inline Instruction(InstructionType type, uint32_t val1, uint32_t val2)
		: op(type) {
		value[0] = val1;
		value[1] = val2;
	}

	inline Instruction(InstructionType type, float val)
		: op(type) {
		value[0] = *(uint32_t*)&val;
		value[1] = 0;
	}

	inline Instruction(InstructionType type, float val1, float val2)
		: op(type) {
		value[0] = *(uint32_t*)&val1;
		value[1] = *(uint32_t*)&val2;
	}
};