#pragma once
#include <QList>
#include "Token.h"
#include "../../Utils/Result.h"

// Converts an original text to a list of tokens
// Omits all the comments found along the way
class Lexer final {
private:
	QList<Token> m_toks;
	std::string m_text;

	std::string m_buffer;
	size_t m_pos = 0;
	size_t m_nextLine = 0;
	size_t m_line = 0;

public:
	Lexer(std::string text);

	utils::Result<QList<Token>> tokenize();
	
private:
	void nextToken();

	void tokenizeNumber();
	void tokenizeOperator();
	void tokenizeWord();

	// reads the line till the end and stores the read text to the last token
	void tokenizeLine();

	void tokenizeComment();

private:
	// reads a word and stores it to -to-
	void loadIdentifier(std::string& to);

	// reads a number and stores it to m_buffer in decimal format
	void loadNumber(int base);

	void skipWhitespaces(bool spacesOnly);

	int isKeyWord(std::string& s);
	bool isOperator(char ch);

	char next();
};