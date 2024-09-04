#include "BuiltInState.h"
#include "ValueExpr.h"
#include "../ByteCodeBuilder.h"

BuiltInState::BuiltInState(TokenType op, std::vector<std::unique_ptr<Expr>> args)
	: m_op(op), m_args(std::move(args)) {
	switch (m_op) {
		case TokenType::SET_IMAGE:
			if (m_args.size() != 1 || !m_args[0]->isConstNumber()) {
				throw QString("Set_image takes one const number argument");
			} else {
				float val = ((ValueExpr*)m_args[0].get())->getConstNumber();
				if (val != floor(val)) {
					throw QString("Set_image's argument must be a whole number!");
				}
			} break;
		case TokenType::SET_COLOR:
			if (m_args.size() != 1 || m_args[0]->getType() != BasicType::COLOR) {
				throw QString("Set_color takes a single color argument");
			} break;
		case TokenType::SET_WIDTH:
			if (m_args.size() != 1 || m_args[0]->getType() != BasicType::NUMBER) {
				throw QString("Set_width takes a single number argument");
			} break;
		case TokenType::COPY_IMAGE:
			if (m_args.size() != 2 || !m_args[0]->isConstNumber() || !m_args[1]->isConstNumber()) {
				throw QString("Copy_image takes 2 const number arguments");
			} else {
				float val1 = ((ValueExpr*)m_args[0].get())->getConstNumber();
				float val2 = ((ValueExpr*)m_args[1].get())->getConstNumber();
				if (val1 != floor(val1) || val2 != floor(val2)) {
					throw QString("Copy_image's arguments must be whole numbers!");
				}
			} break;
			break;
		case TokenType::UPDATE:
			if (m_args.size() != 0) {
				throw QString("Update has no arguments");
			} break;
		case TokenType::DRAW_PIX:
			if (m_args.size() != 1 || m_args[0]->getType() != BasicType::POINT) {
				throw QString("Draw_pix takes a single point argument");
			} break;
		case TokenType::DRAW_STROKE:
			if (m_args.size() != 2 || m_args[0]->getType() != BasicType::POINT || m_args[1]->getType() != BasicType::NUMBER) {
				throw QString("Draw_stroke takes a point and a number");
			} break;
		case TokenType::DRAW_LINE:
			if (m_args.size() != 2 || m_args[0]->getType() != BasicType::POINT || m_args[1]->getType() != BasicType::POINT) {
				throw QString("Draw_line takes 2 points");
			} break;
		case TokenType::DRAW_RECT:
			if (m_args.size() != 2 || m_args[0]->getType() != BasicType::POINT || m_args[1]->getType() != BasicType::POINT) {
				throw QString("Draw_rect takes 2 points");
			} break;
		case TokenType::DRAW_CIRCLE:
			if (m_args.size() != 2 || m_args[0]->getType() != BasicType::POINT || m_args[1]->getType() != BasicType::NUMBER) {
				throw QString("Draw_circle takes a point and a number");
			} break;
		case TokenType::FILL_RECT:
			if (m_args.size() != 2 || m_args[0]->getType() != BasicType::POINT || m_args[1]->getType() != BasicType::POINT) {
				throw QString("Fill_rect takes 2 points");
			} break;
		case TokenType::FILL_CIRCLE:
			if (m_args.size() != 2 || m_args[0]->getType() != BasicType::POINT || m_args[1]->getType() != BasicType::NUMBER) {
				throw QString("Fill_circle takes a point and a number");
			} break;
		case TokenType::SLEEP:
			if (m_args.size() != 1 || m_args[0]->getType() != BasicType::NUMBER) {
				throw QString("Sleep takes a single number argument");
			} break;
	default: throw QString("Not a built-in statement");
	}
}

void BuiltInState::generate() {
	if (m_op == TokenType::SET_IMAGE) {
		uint32_t val = (uint32_t)((ValueExpr*)m_args[0].get())->getConstNumber();
		g_builder.addInst(Instruction(InstructionType::SET_IMAGE, val));
		return;
	} if (m_op == TokenType::COPY_IMAGE) {
		uint32_t val1 = (uint32_t)((ValueExpr*)m_args[0].get())->getConstNumber();
		uint32_t val2 = (uint32_t)((ValueExpr*)m_args[1].get())->getConstNumber();
		g_builder.addInst(Instruction(InstructionType::COPY_IMAGE, val1, val2));
		return;
	}

	for (auto& arg : m_args) {
		arg->generate();
	}

	switch (m_op) {
		case TokenType::SET_COLOR:
			g_builder.addInst(Instruction(InstructionType::SET_COLOR));
			break;
		case TokenType::SET_WIDTH:
			g_builder.addInst(Instruction(InstructionType::SET_WIDTH));
			break;
		case TokenType::UPDATE:
			g_builder.addInst(Instruction(InstructionType::UPDATE));
			break;
		case TokenType::DRAW_PIX:
			g_builder.addInst(Instruction(InstructionType::DRAW_PIX));
			break;
		case TokenType::DRAW_STROKE:
			g_builder.addInst(Instruction(InstructionType::DRAW_STROKE));
			break;
		case TokenType::DRAW_LINE:
			g_builder.addInst(Instruction(InstructionType::DRAW_LINE));
			break;
		case TokenType::DRAW_RECT:
			g_builder.addInst(Instruction(InstructionType::DRAW_RECT));
			break;
		case TokenType::DRAW_CIRCLE:
			g_builder.addInst(Instruction(InstructionType::DRAW_CIRCLE));
			break;
		case TokenType::FILL_RECT:
			g_builder.addInst(Instruction(InstructionType::FILL_RECT));
			break;
		case TokenType::FILL_CIRCLE:
			g_builder.addInst(Instruction(InstructionType::FILL_CIRCLE));
			break;
		case TokenType::SLEEP:
			g_builder.addInst(Instruction(InstructionType::SLEEP));
			break;
	default: throw QString("Not a built-in statement");
	}
}
