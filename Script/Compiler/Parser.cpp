#include "Parser.h"
#include "ast/BinaryExpr.h"
#include "ast/BuiltInExpr.h"
#include "ast/FieldAccessExpr.h"
#include "ast/FunctionCallExpr.h"
#include "ast/UnaryExpr.h"
#include "ast/ValueExpr.h"
#include "ast/VariableExpr.h"
#include "ast/BlockState.h"
#include "ast/BuiltInState.h"
#include "ast/ExprState.h"
#include "ast/ForState.h"
#include "ast/FunctionDefState.h"
#include "ast/IfElseState.h"
#include "ast/SetState.h"
#include "ast/TerminatorState.h"
#include "ast/VariableDeclState.h"
#include "ast/WhileState.h"


static Token _NO_TOK = Token();

Parser::Parser(QList<Token> tokens)
	: m_toks(std::move(tokens)) {

}

utils::Result<std::vector<std::unique_ptr<State>>> Parser::parse() {
	std::vector<std::unique_ptr<State>> result;

	try {
		while (m_pos < m_toks.size()) {
			if (auto state = stateOrBlock(true); state != nullptr) {
				result.push_back(std::move(state));
			}
		}
	} catch (QString error) {
		return utils::Failure(error);
	}

	return result;
}

std::unique_ptr<State> Parser::stateOrBlock(bool isHighLevel) {
	if (match(TokenType::LBRACE)) {
		g_builder.addScope(false);
		std::vector<std::unique_ptr<State>> states;
		while (!match(TokenType::RBRACE)) {
			states.push_back(statement());
		}

		g_builder.deleteScope(false);
		return std::make_unique<BlockState>(std::move(states));
	} else {
		return statement(isHighLevel);
	}
}

std::unique_ptr<State> Parser::statement(bool isHighLevel) {
	if (match(TokenType::IF)) {
		return ifElseStatement();
	} else if (match(TokenType::WHILE)) {
		return whileStatement();
	} else if (match(TokenType::FOR)) {
		return forStatement();
	} else if (peek().type == TokenType::LBRACE) {
		return stateOrBlock(isHighLevel);
	} else if (peek().type == TokenType::LET || peek().type == TokenType::GLOBAL) {
		return variableDeclStatement();
	} else if (match(TokenType::DEF)) {
		return functionDefStatement();
	}

	if (match(TokenType::SET)) {
		std::string name = consume(TokenType::WORD).data;
		SetState::Field field = SetState::NONE;
		if (match(TokenType::DOT)) {
			std::string fieldName = consume(TokenType::WORD).data;

			if (fieldName == "r" || fieldName == "red") {
				field = SetState::RED;
			} else if (fieldName == "g" || fieldName == "green") {
				field = SetState::GREEN;
			} else if (fieldName == "g" || fieldName == "blue") {
				field = SetState::BLUE;
			} else if (fieldName == "a" || fieldName == "alpha") {
				field = SetState::ALPHA;
			} else if (fieldName == "x") {
				field = SetState::X;
			} else if (fieldName == "y") {
				field = SetState::Y;
			} else {
				throw QString("Unknown field name: " + QString::fromStdString(fieldName));
			}
		}

		consume(TokenType::TO);
		return std::make_unique<SetState>(std::move(name), field, expression());
	} if (match(TokenType::CALL)) { // Supposed to be used with functions, but any kind of expression can be used
		return std::make_unique<ExprState>(expression());
	} if (match(TokenType::SET_IMAGE_BUFFER)) {
		uint32_t amount = (uint32_t)std::stoul(consume(TokenType::NUMBER_VAL).data);

		g_builder.setImageBuffersCount(amount);

		return nullptr;
	} if (matchRange(TokenType::SET_IMAGE, TokenType::SLEEP)) {
		TokenType tokenType = peek(-1).type;
		std::vector<std::unique_ptr<Expr>> args;
		
		switch (tokenType) {
			case TokenType::SET_IMAGE:
			case TokenType::SET_COLOR:
			case TokenType::SET_WIDTH:
			case TokenType::SLEEP:
				args.push_back(expression());
				break;
			case TokenType::COPY_IMAGE:
				args.push_back(expression());
				consume(TokenType::TO);
				args.push_back(expression());
				break;
			case TokenType::DRAW_PIX:
			case TokenType::DRAW_STROKE:
			case TokenType::DRAW_LINE:
			case TokenType::DRAW_RECT:
			case TokenType::DRAW_CIRCLE:
			case TokenType::FILL_RECT:
			case TokenType::FILL_CIRCLE:
				args.push_back(expression());
				consume(TokenType::COMMA);
				args.push_back(expression());
				break;
			case TokenType::UPDATE:
		default: break;
		}

		return std::make_unique<BuiltInState>(tokenType, std::move(args));
	}
	
	if (!isHighLevel) {
		if (match(TokenType::RETURN)) {
			return std::make_unique<TerminatorState>(TerminatorAdded::RETURN, expression());
		} if (match(TokenType::BREAK)) {
			return std::make_unique<TerminatorState>(TerminatorAdded::BREAK, expression());
		} if (match(TokenType::CONTINUE)) {
			return std::make_unique<TerminatorState>(TerminatorAdded::CONTINUE, expression());
		}
	}

	throw QString("No statement");
}

std::unique_ptr<State> Parser::functionDefStatement() {
	std::string name = consume(TokenType::WORD).data;

	// Arguments
	g_builder.addScope(false);
	consume(TokenType::LPAR);

	QList<Variable> args;
	Type argTypes;
	if (!match(TokenType::RPAR)) {
		do {
			if (match(TokenType::POINT)) {
				argTypes.push_back(BasicType::POINT);
			} else if (match(TokenType::COLOR)) {
				argTypes.push_back(BasicType::COLOR);
			} else if (match(TokenType::NUMBER)) {
				argTypes.push_back(BasicType::NUMBER);
			} else {
				throw QString("Unstated argument type");
			}

			std::string argName = consume(TokenType::WORD).data;
			args.push_back(g_builder.addVariable(std::move(argName), Type({ argTypes.back() }), false));
		} while (match(TokenType::COMMA));
		consume(TokenType::RPAR);
	}

	Type returnType;

	if (matchRange(TokenType::NUMBER, TokenType::COLOR)) {
		returnType.push_back(BasicType((uint8_t)peek(-1).type - (uint8_t)TokenType::NUMBER + (uint8_t)BasicType::NUMBER));

		while (match(TokenType::COMMA)) {
			returnType.push_back(BasicType(
				(uint8_t)consumeRange(TokenType::NUMBER, TokenType::COLOR).type
				- (uint8_t)TokenType::NUMBER
				+ (uint8_t)BasicType::NUMBER
			));
		}
	}

	// Short function, like def a() number = 10;
	if (match(TokenType::EQ)) {
		std::unique_ptr<Expr> expr = expression();
		std::unique_ptr<State> body = std::make_unique<TerminatorState>(TerminatorAdded::RETURN, std::move(expr));

		g_builder.deleteScope(false);
		return std::make_unique<FunctionDefState>(std::move(name), std::move(returnType), std::move(args), std::move(argTypes), std::move(body));
	} else if (match(TokenType::LBRACE)) {
		m_pos--;
		std::unique_ptr<State> body = stateOrBlock();

		g_builder.deleteScope(false);
		return std::make_unique<FunctionDefState>(std::move(name), std::move(returnType), std::move(args), std::move(argTypes), std::move(body));
	} else {
		g_builder.deleteScope(false);
		return nullptr;
	}
}

std::unique_ptr<State> Parser::variableDeclStatement() {
	bool isGlobal;
	if (match(TokenType::GLOBAL)) {
		isGlobal = true;
	} else {
		consume(TokenType::LET);
		isGlobal = false;
	}

	Type type;
	if (match(TokenType::POINT)) {
		type.push_back(BasicType::POINT);
	} else if (match(TokenType::COLOR)) {
		type.push_back(BasicType::COLOR);
	} else if (match(TokenType::NUMBER)) {
		type.push_back(BasicType::NUMBER);
	} else {
		throw QString("Unstated variable type");
	}

	std::string alias = consume(TokenType::WORD).data;

	consume(TokenType::EQ);
	std::unique_ptr<Expr> expr = expression();

	return std::make_unique<VariableDeclState>(std::move(alias), std::move(type), isGlobal, std::move(expr));
}

std::unique_ptr<State> Parser::forStatement() {
	std::string iterName = consume(TokenType::WORD).data;
	g_builder.addScope(false);
	g_builder.addVariable(iterName, Type({ BasicType::NUMBER }), false);

	consume(TokenType::IN);
	consume(TokenType::RANGE);

	consume(TokenType::LPAR);
	std::unique_ptr<Expr> toExpr = expression();
	std::unique_ptr<Expr> fromExpr = nullptr;
	std::unique_ptr<Expr> stepExpr = nullptr;

	if (match(TokenType::COMMA)) {
		fromExpr = expression();
		std::swap(toExpr, fromExpr);

		if (match(TokenType::COMMA)) {
			stepExpr = expression();
		}
	}
	
	consume(TokenType::RPAR);

	std::unique_ptr<State> body = stateOrBlock();
	
	g_builder.deleteScope(false);
	return std::make_unique<ForState>(std::move(iterName), std::move(fromExpr), std::move(toExpr), std::move(stepExpr), std::move(body));
}

std::unique_ptr<State> Parser::whileStatement() {
	std::unique_ptr<Expr> condition = expression();
	g_builder.addScope(false);

	std::unique_ptr<State> body = stateOrBlock();

	g_builder.deleteScope(false);
	return std::make_unique<WhileState>(std::move(body), std::move(condition));
}

std::unique_ptr<State> Parser::ifElseStatement() {
	std::unique_ptr<Expr> condition = expression();
	g_builder.addScope(false);

	std::unique_ptr<State> body = stateOrBlock();
	std::unique_ptr<State> elseBody = nullptr;

	g_builder.deleteScope(false);

	if (match(TokenType::ELSE)) {
		g_builder.addScope(false);
		elseBody = stateOrBlock();
		g_builder.deleteScope(false);
	}

	return std::make_unique<IfElseState>(std::move(condition), std::move(body), std::move(elseBody));
}

std::unique_ptr<Expr> Parser::expression() {
	return logical();
}

std::unique_ptr<Expr> Parser::logical() {
	std::unique_ptr<Expr> result = conditional();

	while (true) {
		if (match(TokenType::ANDAND)) {
			result = std::make_unique<BinaryExpr>(TokenType::ANDAND, std::move(result), conditional());
			continue;
		} if (match(TokenType::OROR)) {
			result = std::make_unique<BinaryExpr>(TokenType::OROR, std::move(result), conditional());
			continue;
		}

		break;
	}

	return result;
}

std::unique_ptr<Expr> Parser::conditional() {
	std::unique_ptr<Expr> result = additive();

	while (true) {
		if (match(TokenType::EQEQ)) {
			result = std::make_unique<BinaryExpr>(TokenType::EQEQ, std::move(result), additive());
			continue;
		} if (match(TokenType::NOT_EQ)) {
			result = std::make_unique<BinaryExpr>(TokenType::NOT_EQ, std::move(result), additive());
			continue;
		} if (match(TokenType::LESS)) {
			result = std::make_unique<BinaryExpr>(TokenType::LESS, std::move(result), additive());
			continue;
		} if (match(TokenType::LESS_EQ)) {
			result = std::make_unique<BinaryExpr>(TokenType::LESS_EQ, std::move(result), additive());
			continue;
		} if (match(TokenType::GREATER)) {
			result = std::make_unique<BinaryExpr>(TokenType::GREATER, std::move(result), additive());
			continue;
		} if (match(TokenType::GREATER_EQ)) {
			result = std::make_unique<BinaryExpr>(TokenType::GREATER_EQ, std::move(result), additive());
			continue;
		}

		break;
	}

	return result;
}

std::unique_ptr<Expr> Parser::additive() {
	std::unique_ptr<Expr> result = multiplicative();

	while (true) {
		if (match(TokenType::PLUS)) {
			result = std::make_unique<BinaryExpr>(TokenType::PLUS, std::move(result), multiplicative());
			continue;
		} if (match(TokenType::MINUS)) {
			result = std::make_unique<BinaryExpr>(TokenType::MINUS, std::move(result), multiplicative());
			continue;
		}

		break;
	}

	return result;
}

std::unique_ptr<Expr> Parser::multiplicative() {
	std::unique_ptr<Expr> result = degree();

	while (true) {
		if (match(TokenType::STAR)) {
			result = std::make_unique<BinaryExpr>(TokenType::STAR, std::move(result), degree());
			continue;
		} if (match(TokenType::SLASH)) {
			result = std::make_unique<BinaryExpr>(TokenType::SLASH, std::move(result), degree());
			continue;
		} if (match(TokenType::PERCENT)) {
			result = std::make_unique<BinaryExpr>(TokenType::PERCENT, std::move(result), degree());
			continue;
		}

		break;
	}

	return result;
}

std::unique_ptr<Expr> Parser::degree() {
	std::unique_ptr<Expr> result = unary();

	while (true) {
		if (match(TokenType::DOUBLE_STAR)) {
			result = std::make_unique<BinaryExpr>(TokenType::DOUBLE_STAR, std::move(result), unary());
			continue;
		}

		break;
	}

	return result;
}

std::unique_ptr<Expr> Parser::unary() {
	while (true) {
		if (match(TokenType::MINUS)) {
			return std::make_unique<UnaryExpr>(TokenType::MINUS, unary());
		} if (match(TokenType::NOT)) {
			return std::make_unique<UnaryExpr>(TokenType::NOT, unary());
		}

		break;
	}

	return postfix();
}

std::unique_ptr<Expr> Parser::postfix() {
	std::unique_ptr<Expr> result = primary();

	while (match(TokenType::DOT)) { // Field access
		consume(TokenType::WORD);

		std::string field = peek(-1).data;

		if (field == "r" || field == "red") {
			result = std::make_unique<FieldAccessExpr>(FieldAccessExpr::RED, std::move(result));
			continue;
		} if (field == "g" || field == "green") {
			result = std::make_unique<FieldAccessExpr>(FieldAccessExpr::GREEN, std::move(result));
			continue;
		} if (field == "g" || field == "blue") {
			result = std::make_unique<FieldAccessExpr>(FieldAccessExpr::BLUE, std::move(result));
			continue;
		} if (field == "a" || field == "alpha") {
			result = std::make_unique<FieldAccessExpr>(FieldAccessExpr::ALPHA, std::move(result));
			continue;
		} if (field == "x") {
			result = std::make_unique<FieldAccessExpr>(FieldAccessExpr::X, std::move(result));
			continue;
		} if (field == "y") {
			result = std::make_unique<FieldAccessExpr>(FieldAccessExpr::Y, std::move(result));
			continue;
		}

		throw QString("Unknown field: " + QString::fromStdString(field));
	}

	return result;
}

std::unique_ptr<Expr> Parser::primary() {
	if (match(TokenType::LPAR)) {
		std::unique_ptr<Expr> result = expression();
		consume(TokenType::RPAR);
		return result;
	}

	// Literals
	if (match(TokenType::NUMBER_VAL)) {
		return std::make_unique<ValueExpr>(std::stof(peek(-1).data));
	}

	// Built-ins
	if (match(TokenType::WIDTH)) {
		return std::make_unique<BuiltInExpr>(TokenType::WIDTH);
	} if (match(TokenType::HEIGHT)) {
		return std::make_unique<BuiltInExpr>(TokenType::HEIGHT);
	}

	if (matchRange(TokenType::ABS, TokenType::DISTANCE) || match(TokenType::COLOR) || match(TokenType::POINT)) {
		std::vector<std::unique_ptr<Expr>> args;
		TokenType op = peek(-1).type;

		consume(TokenType::LPAR);
		if (!match(TokenType::RPAR)) {
			do {
				args.push_back(expression());
			} while (match(TokenType::COMMA));

			consume(TokenType::RPAR);
		}

		return std::make_unique<BuiltInExpr>(op, std::move(args));
	}

	// Identifier
	if (match(TokenType::WORD)) {
		std::string name = peek(-1).data;

		if (match(TokenType::LPAR)) { // Function call
			std::vector<std::unique_ptr<Expr>> args;

			if (!match(TokenType::RPAR)) {
				do {
					args.push_back(expression());
				} while (match(TokenType::COMMA));

				consume(TokenType::RPAR);
			}

			return std::make_unique<FunctionCallExpr>(name, std::move(args));
		} else { // Variable
			return std::make_unique<VariableExpr>(name);
		}
	}

	throw QString("Unknown expression");
	// return nullptr;
}

Token& Parser::consume(TokenType type) {
	if (!match(type)) {
		throw QString("Unexpected token");
	}

	return peek(-1);
}

Token& Parser::consumeRange(TokenType from, TokenType to) {
	if (!matchRange(from, to)) {
		throw QString("Unexpected token");
	}

	return peek(-1);
}

bool Parser::match(TokenType type) {
	if (peek().type != type) {
		return false;
	}

	m_pos++;
	return true;
}

bool Parser::matchRange(TokenType from, TokenType to) {
	TokenType type = peek().type;
	if (type < from || type > to) {
		return false;
	}

	m_pos++;
	return true;
}

Token& Parser::next() {
	if (m_pos >= m_toks.size()) {
		return _NO_TOK;
	}

	return m_toks[m_pos++];
}

Token& Parser::peek(int rel) {
	size_t pos = m_pos + rel;
	if (pos >= m_toks.size()) {
		return _NO_TOK;
	}

	return m_toks[pos];
}

int Parser::getCurrLine() {
	Token& tok = peek();
	if (tok.type == TokenType::NO_TOKEN) {
		return m_toks.back().errLine;
	}

	return tok.errLine;
}
