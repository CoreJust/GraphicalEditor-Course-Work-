#include "FieldAccessExpr.h"
#include "../ByteCodeBuilder.h"

FieldAccessExpr::FieldAccessExpr(Field field, std::unique_ptr<Expr> expr)
	: m_field(field), m_expr(std::move(expr)) {
	BasicType exprType = m_expr->getType()[0];
	if (field == Field::RED || field == Field::GREEN || field == Field::BLUE || field == Field::ALPHA) {
		if (exprType != BasicType::COLOR) {
			throw QString("Only colors can have rgba fields");
		}
	} else if (field == Field::X || field == Field::Y) {
		if (exprType != BasicType::POINT) {
			throw QString("Only points can have xy fields");
		}
	}

	m_type = Type({ BasicType::NUMBER });
}

void FieldAccessExpr::generate() {
	m_expr->generate();

	switch (m_field) {
	case FieldAccessExpr::RED:
		g_builder.addInst(Instruction(InstructionType::GET_COLOR_RED));
		break;
	case FieldAccessExpr::GREEN:
		g_builder.addInst(Instruction(InstructionType::GET_COLOR_GREEN));
		break;
	case FieldAccessExpr::BLUE:
		g_builder.addInst(Instruction(InstructionType::GET_COLOR_BLUE));
		break;
	case FieldAccessExpr::ALPHA:
		g_builder.addInst(Instruction(InstructionType::GET_COLOR_ALPHA));
		break;
	case FieldAccessExpr::X:
		g_builder.addInst(Instruction(InstructionType::GET_POINT_X));
		break;
	case FieldAccessExpr::Y:
		g_builder.addInst(Instruction(InstructionType::GET_POINT_Y));
		break;
	default:
		break;
	}
}

bool FieldAccessExpr::isLVal() {
	return m_expr->isLVal();
}
