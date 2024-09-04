#pragma once
#include <memory>
#include <QList>
#include "../../Utils/Result.h"
#include "Token.h"
#include "ast/State.h"
#include "ast/Expr.h"

// Parser a list of tokens into an Abstact Syntax Tree
class Parser final {
protected:
	QList<Token> m_toks;
	size_t m_pos = 0;

public:
	Parser(QList<Token> tokens);

	// std::vector is used instead of QList because QList cannot handle unique_ptr
	utils::Result<std::vector<std::unique_ptr<State>>> parse();

private:
	// Parsing the tokens by recursive descent
	std::unique_ptr<State> stateOrBlock(bool isHighLevel = false);
	std::unique_ptr<State> statement(bool isHighLevel = false);
	std::unique_ptr<State> functionDefStatement();
	std::unique_ptr<State> variableDeclStatement();
	std::unique_ptr<State> forStatement();
	std::unique_ptr<State> whileStatement();
	std::unique_ptr<State> ifElseStatement();

	std::unique_ptr<Expr> expression();
	std::unique_ptr<Expr> logical();
	std::unique_ptr<Expr> conditional();
	std::unique_ptr<Expr> additive();
	std::unique_ptr<Expr> multiplicative();
	std::unique_ptr<Expr> degree();
	std::unique_ptr<Expr> unary();
	std::unique_ptr<Expr> postfix();
	std::unique_ptr<Expr> primary();

private:
	Token& consume(TokenType type);
	Token& consumeRange(TokenType from, TokenType to);

	bool match(TokenType type);
	bool matchRange(TokenType from, TokenType to);

	Token& next();
	Token& peek(int rel = 0);

	int getCurrLine();
};