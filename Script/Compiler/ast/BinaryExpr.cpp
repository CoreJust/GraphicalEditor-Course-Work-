#include "BinaryExpr.h"
#include "../ByteCodeBuilder.h"

BinaryExpr::BinaryExpr(TokenType op, std::unique_ptr<Expr> left, std::unique_ptr<Expr> right)
	: Expr(getCommonType(left->getType(), right->getType())), m_op(op), m_left(std::move(left)), m_right(std::move(right)) {
	if (op == TokenType::EQEQ || op == TokenType::NOT_EQ || op == TokenType::LESS
		|| op == TokenType::GREATER || op == TokenType::LESS_EQ || op == TokenType::GREATER_EQ) {
		if (m_left->getType() != m_right->getType()) {
			throw QString("Must be the same type");
		}

		if (op == TokenType::LESS || op == TokenType::GREATER || op == TokenType::LESS_EQ || op == TokenType::GREATER_EQ) {
			if (m_left->getType()[0] != BasicType::NUMBER) {
				throw QString("Only numbers can be compared to each other");
			}
		}
	} else if (op == TokenType::OROR || op == TokenType::ANDAND) {
		if (m_left->getType() != m_right->getType() || m_left->getType()[0] != BasicType::NUMBER) {
			throw QString("Must be a number");
		}
	}
}

void BinaryExpr::generate() {
	switch (m_op) {
		case TokenType::PLUS:
			if (m_left->getType() == m_right->getType()) {
				m_left->generate();
				m_right->generate();
				g_builder.addInst(Instruction(getAccordingToType(InstructionType::ADD, (uint8_t)m_left->getType()[0])));
			} else {
				if (m_left->getType()[0] == BasicType::NUMBER) {
					m_right->generate();
					m_left->generate();
					g_builder.addInst(Instruction(
						getAccordingToType(
							InstructionType((int)InstructionType::ADD_TO_POINT - 1), 
							(uint8_t)m_right->getType()[0]
						)
					));
				} else if (m_right->getType()[0] == BasicType::NUMBER) {
					m_left->generate();
					m_right->generate();
					g_builder.addInst(Instruction(
						getAccordingToType(
							InstructionType((int)InstructionType::ADD_TO_POINT - 1),
							(uint8_t)m_left->getType()[0]
						)
					));
				} else {
					throw QString("Wrong types for addition");
				}
			} break;
		case TokenType::MINUS:
			if (m_left->getType() == m_right->getType()) {
				m_left->generate();
				m_right->generate();
				g_builder.addInst(Instruction(getAccordingToType(InstructionType::SUB, (uint8_t)m_left->getType()[0])));
			} else {
				if (m_right->getType()[0] == BasicType::NUMBER) {
					m_left->generate();
					m_right->generate();
					g_builder.addInst(Instruction(
						getAccordingToType(
							InstructionType((int)InstructionType::SUB_FROM_POINT - 1),
							(uint8_t)m_left->getType()[0]
						)
					));
				} else {
					throw QString("Wrong types for substaction");
				}
			} break;
		case TokenType::STAR:
			if (m_left->getType()[0] == BasicType::NUMBER) {
				m_right->generate();
				m_left->generate();
				g_builder.addInst(Instruction(
					getAccordingToType(
						InstructionType::MUL,
						(uint8_t)m_right->getType()[0]
					)
				));
			} else if (m_right->getType()[0] == BasicType::NUMBER) {
				m_left->generate();
				m_right->generate();
				g_builder.addInst(Instruction(
					getAccordingToType(
						InstructionType::MUL,
						(uint8_t)m_left->getType()[0]
					)
				));
			} else {
				throw QString("Wrong types for multiplication");
			} break;
		case TokenType::SLASH:
			if (m_right->getType()[0] == BasicType::NUMBER) {
				m_left->generate();
				m_right->generate();
				g_builder.addInst(Instruction(
					getAccordingToType(
						InstructionType::DIV,
						(uint8_t)m_left->getType()[0]
					)
				));
			} else {
				throw QString("Wrong types for division");
			} break;
		case TokenType::PERCENT:
			if (m_right->getType()[0] == BasicType::NUMBER) {
				m_left->generate();
				m_right->generate();
				g_builder.addInst(Instruction(
					getAccordingToType(
						InstructionType::MOD,
						(uint8_t)m_left->getType()[0]
					)
				));
			} else {
				throw QString("Wrong types for division");
			} break;
		case TokenType::DOUBLE_STAR:
			if (m_right->getType()[0] == BasicType::NUMBER) {
				m_left->generate();
				m_right->generate();
				g_builder.addInst(Instruction(
					getAccordingToType(
						InstructionType::POW,
						(uint8_t)m_left->getType()[0]
					)
				));
			} else {
				throw QString("Wrong types for division");
			} break;

		// For the next ones types are already verified
		case TokenType::EQEQ:
			m_left->generate();
			m_right->generate();
			g_builder.addInst(Instruction(getAccordingToType(
				InstructionType::CMP_EQ,
				(uint8_t)m_right->getType()[0]
			)));
			break;
		case TokenType::NOT_EQ:
			m_left->generate();
			m_right->generate();
			g_builder.addInst(Instruction(getAccordingToType(
				InstructionType::CMP_NEQ,
				(uint8_t)m_right->getType()[0]
			)));
			break;
		case TokenType::LESS:
			m_left->generate();
			m_right->generate();
			g_builder.addInst(Instruction(InstructionType::CMP_LT));
			break;
		case TokenType::GREATER:
			m_left->generate();
			m_right->generate();
			g_builder.addInst(Instruction(InstructionType::CMP_GT));
			break;
		case TokenType::LESS_EQ:
			m_left->generate();
			m_right->generate();
			g_builder.addInst(Instruction(InstructionType::CMP_LE));
			break;
		case TokenType::GREATER_EQ:
			m_left->generate();
			m_right->generate();
			g_builder.addInst(Instruction(InstructionType::CMP_GE));
			break;
		case TokenType::ANDAND:
			m_left->generate();
			m_right->generate();
			g_builder.addInst(Instruction(InstructionType::AND));
			break;
		case TokenType::OROR:
			m_left->generate();
			m_right->generate();
			g_builder.addInst(Instruction(InstructionType::OR));
			break;
	default: throw QString("Not a binary operator");
	}
}

bool BinaryExpr::isLVal() {
	return false;
}
