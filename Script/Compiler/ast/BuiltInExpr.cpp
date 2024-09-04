#include "BuiltInExpr.h"
#include "../ByteCodeBuilder.h"

BuiltInExpr::BuiltInExpr(TokenType op)
	: BuiltInExpr(op, std::vector<std::unique_ptr<Expr>>()) {

}

BuiltInExpr::BuiltInExpr(TokenType op, std::vector<std::unique_ptr<Expr>> args)
    : m_op(op), m_args(std::move(args)) {
	switch (m_op) {
	case TokenType::COLOR:
		if (m_args.size() != 4 || m_args[0]->getType() != BasicType::NUMBER || m_args[1]->getType() != BasicType::NUMBER
			|| m_args[2]->getType() != BasicType::NUMBER || m_args[3]->getType() != BasicType::NUMBER) {
			throw QString("Color must have 4 number arguments");
		}

		m_type = Type({ BasicType::COLOR });
		break;
	case TokenType::POINT:
		if (m_args.size() != 2 || m_args[0]->getType() != BasicType::NUMBER || m_args[1]->getType() != BasicType::NUMBER) {
			throw QString("Point must have 2 number arguments");
		}

		m_type = Type({ BasicType::POINT });
		break;
	case TokenType::WIDTH:
		if (m_args.size() != 0) {
			throw QString("Width has no arguments");
		}

		m_type = Type({ BasicType::NUMBER });
		break;
	case TokenType::HEIGHT:
		if (m_args.size() != 0) {
			throw QString("Height has no arguments");
		}

		m_type = Type({ BasicType::NUMBER });
		break;
	case TokenType::ABS:
		if (m_args.size() != 1 || m_args[0]->getType() != BasicType::NUMBER) {
			throw QString("Abs takes 1 number argument");
		}

		m_type = Type({ BasicType::NUMBER });
		break;
	case TokenType::MIN:
		if (m_args.size() < 1) {
			throw QString("Min takes at least 1 arguments");
		}

		for (auto& arg : m_args) {
			if (arg->getType() != BasicType::NUMBER) {
				throw QString("Min takes only number arguments");
			}
		}

		m_type = Type({ BasicType::NUMBER });
		break;
	case TokenType::MAX:
		if (m_args.size() < 1) {
			throw QString("Max takes at least 1 arguments");
		}

		for (auto& arg : m_args) {
			if (arg->getType() != BasicType::NUMBER) {
				throw QString("Max takes only number arguments");
			}
		}

		m_type = Type({ BasicType::NUMBER });
		break;
	case TokenType::SUM:
		if (m_args.size() < 1) {
			throw QString("Sum takes at least 1 arguments");
		}

		for (auto& arg : m_args) {
			if (arg->getType() != BasicType::NUMBER) {
				throw QString("Sum takes only number arguments");
			}
		}

		m_type = Type({ BasicType::NUMBER });
		break;
	case TokenType::ROUND:
		if (m_args.size() != 1 || m_args[0]->getType() != BasicType::NUMBER) {
			throw QString("Round takes 1 number argument");
		}

		m_type = Type({ BasicType::NUMBER });
		break;
	case TokenType::FLOOR:
		if (m_args.size() != 1 || m_args[0]->getType() != BasicType::NUMBER) {
			throw QString("Floor takes 1 number argument");
		}

		m_type = Type({ BasicType::NUMBER });
		break;
	case TokenType::CEIL:
		if (m_args.size() != 1 || m_args[0]->getType() != BasicType::NUMBER) {
			throw QString("Ceil takes 1 number argument");
		}

		m_type = Type({ BasicType::NUMBER });
		break;
	case TokenType::SIN:
		if (m_args.size() != 1 || m_args[0]->getType() != BasicType::NUMBER) {
			throw QString("Sin takes 1 number argument");
		}

		m_type = Type({ BasicType::NUMBER });
		break;
	case TokenType::COS:
		if (m_args.size() != 1 || m_args[0]->getType() != BasicType::NUMBER) {
			throw QString("Cos takes 1 number argument");
		}

		m_type = Type({ BasicType::NUMBER });
		break;
	case TokenType::TAN:
		if (m_args.size() != 1 || m_args[0]->getType() != BasicType::NUMBER) {
			throw QString("Tan takes 1 number argument");
		}

		m_type = Type({ BasicType::NUMBER });
		break;
	case TokenType::COT:
		if (m_args.size() != 1 || m_args[0]->getType() != BasicType::NUMBER) {
			throw QString("Cot takes 1 number argument");
		}

		m_type = Type({ BasicType::NUMBER });
		break;
	case TokenType::EXP:
		if (m_args.size() != 1 || m_args[0]->getType() != BasicType::NUMBER) {
			throw QString("Exp takes 1 number argument");
		}

		m_type = Type({ BasicType::NUMBER });
		break;
	case TokenType::LOG:
		if (m_args.size() != 1 || m_args[0]->getType() != BasicType::NUMBER) {
			throw QString("Log takes 1 number argument");
		}

		m_type = Type({ BasicType::NUMBER });
		break;
	case TokenType::LENGTH:
		if (m_args.size() != 1 || m_args[0]->getType() != BasicType::POINT) {
			throw QString("Length takes 1 point argument");
		}

		m_type = Type({ BasicType::NUMBER });
		break;
	case TokenType::DISTANCE:
		if (m_args.size() != 2 || m_args[0]->getType() != BasicType::POINT || m_args[1]->getType() != BasicType::POINT) {
			throw QString("Distance takes 2 point argument");
		}

		m_type = Type({ BasicType::NUMBER });
		break;
	default: throw QString("Not a built-in expression's token");
	}
}

void BuiltInExpr::generate() {
	for (auto& arg : m_args) {
		arg->generate();
	}

	switch (m_op) {
	case TokenType::COLOR: // Actually does nothing, just a type conversion
		break;
	case TokenType::POINT: // Actually does nothing, just a type conversion
		break;
	case TokenType::WIDTH:
		g_builder.addInst(Instruction(InstructionType::PUSH_WIDTH));
		break;
	case TokenType::HEIGHT:
		g_builder.addInst(Instruction(InstructionType::PUSH_HEIGHT));
		break;
	case TokenType::ABS:
		g_builder.addInst(Instruction(InstructionType::ABS));
		break;
	case TokenType::MIN:
		g_builder.addInst(Instruction(InstructionType::MIN, uint32_t(m_args.size())));
		break;
	case TokenType::MAX:
		g_builder.addInst(Instruction(InstructionType::MAX, uint32_t(m_args.size())));
		break;
	case TokenType::SUM:
		g_builder.addInst(Instruction(InstructionType::SUM, uint32_t(m_args.size())));
		break;
	case TokenType::ROUND:
		g_builder.addInst(Instruction(InstructionType::ROUND));
		break;
	case TokenType::FLOOR:
		g_builder.addInst(Instruction(InstructionType::FLOOR));
		break;
	case TokenType::CEIL:
		g_builder.addInst(Instruction(InstructionType::CEIL));
		break;
	case TokenType::SIN:
		g_builder.addInst(Instruction(InstructionType::SIN));
		break;
	case TokenType::COS:
		g_builder.addInst(Instruction(InstructionType::COS));
		break;
	case TokenType::TAN:
		g_builder.addInst(Instruction(InstructionType::TAN));
		break;
	case TokenType::COT:
		g_builder.addInst(Instruction(InstructionType::COT));
		break;
	case TokenType::EXP:
		g_builder.addInst(Instruction(InstructionType::EXP));
		break;
	case TokenType::LOG:
		g_builder.addInst(Instruction(InstructionType::LOG));
		break;
	case TokenType::LENGTH:
		g_builder.addInst(Instruction(InstructionType::LENGTH));
		break;
	case TokenType::DISTANCE:
		g_builder.addInst(Instruction(InstructionType::DISTANCE));
		break;
	default: throw QString("Not a built-in expression's token");
	}
}

bool BuiltInExpr::isLVal() {
    return false;
}
