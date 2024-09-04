#include "Lexer.h"
#include <QMap>
#include <functional>
#include <QMessageBox>

QList<std::string> KEY_WORDS{
	"let", "global", "set", "call", "Number", "Point", "Color",
	"def", "if", "else", "while", "for",
	"continue", "break", "return", "to", "in", "range",
	"set_image_buffer", "set_image", "set_color", "set_width", "copy_image", "update",
	"draw_pix", "draw_stroke", "draw_line", "draw_rect", "draw_circle",
	"fill_rect", "fill_circle",
	"sleep",
	"Width", "Height",
	"abs", "min", "max", "sum",
	"round", "floor", "ceil",
	"sin", "cos", "tan", "cot",
	"exp", "log", "length", "distance"
};

QMap<std::string, TokenType> OPERATORS = {
	{ "=", TokenType::EQ },
	{ "+", TokenType::PLUS },
	{ "-", TokenType::MINUS },
	{ "*", TokenType::STAR },
	{ "/", TokenType::SLASH },
	{ "%", TokenType::PERCENT },
	{ "**", TokenType::DOUBLE_STAR },

	{ "!", TokenType::NOT },
	{ "==", TokenType::EQEQ },
	{ "!=", TokenType::NOT_EQ },
	{ "<", TokenType::LESS },
	{ ">", TokenType::GREATER },
	{ "<=", TokenType::LESS_EQ },
	{ ">=", TokenType::GREATER_EQ },

	{ "&&", TokenType::ANDAND },
	{ "||", TokenType::OROR },

	{ "(", TokenType::LPAR },
	{ ")", TokenType::RPAR },

	{ "{", TokenType::LBRACE },
	{ "}", TokenType::RBRACE },

	{ ",", TokenType::COMMA },
	{ ".", TokenType::DOT },

	{ ";", TokenType::SEMICOLON }
};

const std::string OP_CHARS = "<>=+-*/%&|!(){},.;";

// Util functions
// Single character to decimal number (e.g. 'a' to 10, '3' to 3)
int charToDecimal(char c) {
	return isdigit(c) ?
		c - '0'
		: tolower(c) - 'a' + 10;
}

// For floating-point numbers
double to_double(const std::string& s, int num_sys) {
	double r = 0.0;
	bool wasPoint = false;
	bool isNeg = false;
	double afterPoint = 1;
	size_t i = 0;

	if (s[0] == '-') {
		isNeg = ++i;
	}

	for (; i < s.size(); ++i) {
		if (s[i] == '.') {
			wasPoint = true;
		} else if (!wasPoint) {
			r *= num_sys;
			r += charToDecimal(s[i]);
		} else {
			afterPoint *= num_sys;
			r += charToDecimal(s[i]) / afterPoint;
		}
	}

	return isNeg ? -r : r;
}

// For whole numbers
uint64_t to_u64(const std::string& s, int num_sys) {
	uint64_t r = 0;
	for (size_t i = 0; i < s.size(); ++i) {
		r *= num_sys;
		r += charToDecimal(s[i]);
	}

	return r;
}

// Converts number in string from some number system to decimal
void toDecimal(std::string& s, int nsys) {
	if (nsys == 10)
		return;

	if (s.find('.') != std::string::npos) {
		s = std::to_string(to_double(s, nsys));
	} else {
		s = std::to_string(to_u64(s, nsys));
	}
}


Lexer::Lexer(std::string text)
	: m_text(std::move(text)) {

}

utils::Result<QList<Token>> Lexer::tokenize() {
	try {
		while (m_pos < m_text.size()) {
			nextToken();
		}
	} catch (QString error) {
		return utils::Failure(error);
	}

	return std::move(m_toks);
}

void Lexer::nextToken() {
	if (isdigit(m_text[m_pos])) {
		tokenizeNumber();
	} else if (isalpha(m_text[m_pos]) || m_text[m_pos] == '_') {
		tokenizeWord();
	} else if (m_text[m_pos] == '#') {
		tokenizeComment();
	} else if (isOperator(m_text[m_pos])) {
		tokenizeOperator();
	} else {
		next();
	}
}

void Lexer::tokenizeNumber() {
	char c = m_text[m_pos];

	// Number system (hex, binary, etc)
	int number_system = 10;
	if (c == '0') {
		c = next();
		switch (c) {
			case 'x': number_system = 16; break;
			case 'o': number_system = 8; break;
			case 'b': number_system = 2; break;
		default: break;
		}

		if (number_system != 10) {
			c = next();
		} else {
			c = m_text[--m_pos];
		}
	}

	loadNumber(number_system);

	m_toks.push_back(Token{ m_buffer, m_line, TokenType::NUMBER_VAL });
}

void Lexer::tokenizeOperator() {
	m_buffer.clear();

	char c = m_text[m_pos];
	while (c != '\0') {
		m_buffer += c;
		if (m_buffer.size() > 1) {
			if (OPERATORS.find(m_buffer) == OPERATORS.end()) {
				m_buffer.erase(m_buffer.size() - 1, 1);
				m_toks.push_back(Token{ m_buffer, m_line, OPERATORS[m_buffer] });
				return;
			}
		}

		c = next();
	}

	// Met the EOF
	if (OPERATORS.find(m_buffer) == OPERATORS.end()) {
		m_buffer.erase(m_buffer.size() - 1, 1);
	}

	if (OPERATORS.find(m_buffer) != OPERATORS.end()) {
		m_toks.push_back(Token{ m_buffer, m_line, OPERATORS[m_buffer] });
	} else {
		throw QString("Operator not found: ") + QString::fromStdString(m_buffer);
	}
}

void Lexer::tokenizeWord() {
	m_buffer.clear();
	loadIdentifier(m_buffer);

	int isKey = isKeyWord(m_buffer);
	if (isKey != -1) {
		TokenType keyWord = TokenType((uint8_t)TokenType::LET + isKey);
		m_toks.push_back(Token{ m_buffer, m_line, keyWord });
	} else {
		m_toks.push_back(Token{ m_buffer, m_line, TokenType::WORD });
	}
}

void Lexer::tokenizeLine() {
	m_buffer.clear();

	char c = next(); // skip whitespace
	while (c != '\n' && c != '\r' && c != '\0') {
		m_buffer += c;
		c = next();
	}

	m_toks.back().data = m_buffer;
}

void Lexer::tokenizeComment() {
	if (m_pos < m_text.size() - 6 && m_text[m_pos + 1] == '#' && m_text[m_pos + 2] == '#') { // multiline comment
		next();
		next();
		next();
		while (true) {
			if (m_pos == m_text.size()) {
				throw QString("Multiline comment is not closed!");
			}

			if (m_pos < m_text.size() - 2) {
				if (m_text[m_pos] == '#' && m_text[m_pos + 1] == '#' && m_text[m_pos + 2] == '#') {
					next();
					next();
					next();
					return;
				}
			}

			next();
		}
	}

	// single-line comment
	char c = next();
	while (c != '\n' && c != '\0' && c != '\r') {
		c = next();
	}
}

void Lexer::loadIdentifier(std::string& to) {
	char c = m_text[m_pos];
	do {
		to += c;
		c = next();
	} while (isalnum(c) || c == '_' || c == '$');
}

void Lexer::loadNumber(int base) {
	int _case = 0; // 1 if lower case, 2 if upper case
	int firstWrongCase = -1;
	auto& pos = m_pos;
	auto binIsDigit = [](char c) -> int { return c == '0' || c == '1'; };
	auto octIsDigit = [](char c) -> int { return c >= '0' && c <= '7'; };
	auto decIsDigit = [](char c) -> int { return isdigit(c); };
	auto hexIsDigit = [&_case, &firstWrongCase, &pos](char c) -> int {
		if (!isdigit(c) && isxdigit(c)) { // number can consist letters only of the same case
			if (!_case) {
				_case = islower(c) ? 1 : 2;
			}
			else {
				if (firstWrongCase == -1 && (islower(c) ? _case == 2 : _case == 1)) {
					firstWrongCase = pos;
				}
			}
		}

		return isxdigit(c);
	};

	std::function<int(char)> isDigit;
	switch (base) {
	case 2: isDigit = binIsDigit; break;
	case 8: isDigit = octIsDigit; break;
	case 10: isDigit = decIsDigit; break;
	case 16: isDigit = hexIsDigit; break;
	default:
		assert(false);
	}

	m_buffer.clear();
	char c = m_text[m_pos];
	do {
		if (c != '\'') {
			m_buffer += c;
		}

		c = next();
	} while (isDigit(c) || (c == '\''));

	// handle floating point like "1.2345"
	if (c == '.') {
		m_buffer += c;
		c = next();
		while (isDigit(c) || (c == '\'')) {
			if (c != '\'') {
				m_buffer += c;
			}

			c = next();
		}
	}

	// check for case consistency and search for a type ending (aka f32/f64).
	if (base == 16) {
		if (m_buffer.size() > 3 && m_buffer[m_buffer.size() - 3] == 'f'
			&& ((m_buffer[m_buffer.size() - 2] == '3' && m_buffer[m_buffer.size() - 1] == '2')
				|| (m_buffer[m_buffer.size() - 2] == '6' && m_buffer[m_buffer.size() - 1] == '4'))
			) {
			m_buffer.resize(m_buffer.size() - 3); // erase the type ending from value section
			m_pos -= 3;
		}
	}

	toDecimal(m_buffer, base);
}

void Lexer::skipWhitespaces(bool spacesOnly) {
	static std::string allSpaces = " \t\n\r\f\v";
	if (spacesOnly) {
		while (m_pos < m_text.size() && (m_text[m_pos] == ' ' || m_text[m_pos] == '\t')) {
			next(); // skip whitespaces
		}
	} else {
		while (m_pos < m_text.size() && allSpaces.find(m_text[m_pos]) != std::string::npos) {
			next(); // skip whitespaces
		}
	}
}

int Lexer::isKeyWord(std::string& s) {
	for (int i = 0; i < KEY_WORDS.size(); i++) {
		if (s == KEY_WORDS[i]) {
			return i;
		}
	}

	return -1;
}

bool Lexer::isOperator(char ch) {
	for (auto c : OP_CHARS) {
		if (ch == c) {
			return true;
		}
	}

	return false;
}

char Lexer::next() {
	m_pos++;
	if (m_line < m_nextLine) {
		m_line = m_nextLine;
	}

	if (m_pos < m_text.size()) {
		char r = m_text[m_pos];
		if (r == '\n' || r == '\r') {
			m_nextLine++;
		}

		return r;
	} else {
		return '\0';
	}
}
