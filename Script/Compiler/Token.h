#pragma once
#include <cstdint>
#include <string>

// CW2 script's token types
enum class TokenType : uint8_t {
	NUMBER_VAL = 0,

	WORD, // any identifier

	// Operators
	EQ, // =
	PLUS, // +
	MINUS, // -
	STAR, // *
	SLASH, // /
	PERCENT, // %
	DOUBLE_STAR, // **

	NOT, // !
	EQEQ, // ==
	NOT_EQ, // !=
	LESS, // <
	GREATER, // >
	LESS_EQ, // <=
	GREATER_EQ, // >=

	ANDAND, // &&
	OROR, // ||

	LPAR, // (
	RPAR, // )

	LBRACE, // {
	RBRACE, // }

	COMMA, // ,
	DOT, // .

	SEMICOLON, // ;

	// Key words
	LET, // let -type- -name- = -init-, creating a local variable
	GLOBAL, // global -type- -name- = -init-,  creating a global variable

	SET, // set -variable- to -value-

	CALL, // call -expr- (supposedly function call)

	NUMBER, // number type
	POINT, // point constuctor/point type
	COLOR, // color constructor/color type

	DEF, // def -name-([-type- arg1[, ...]]) [return-type] ...

	IF,
	ELSE,
	WHILE,
	FOR, // for -name- in range(-from: number-, -to: number-, -step: number-) ...

	CONTINUE,
	BREAK,
	RETURN,

	TO,
	IN,
	RANGE,

	// Image-related tokens
	SET_IMAGE_BUFFER, // set_image_buffer -amount: constant number- (2 by default), can be put in the beginning and executed only once
	SET_IMAGE, // set_image -id: constant number-

	SET_COLOR, // set_color -color: color-
	SET_WIDTH, // set_width -width: number-

	COPY_IMAGE, // copy_image -from: constant number- to -to: constant number-
	UPDATE, // Displaying changes

	// Drawing functions
	DRAW_PIX, // draw_pix -pos: point-
	DRAW_STROKE, // draw_stroke -pos: point-, -length: number-
	DRAW_LINE, // draw_line -from: point-, -to: point-
	DRAW_RECT, // draw_rect -from: point-, -to: point-
	DRAW_CIRCLE, // draw_circle -center: point-, -radius: number-

	FILL_RECT, // fill_rect -from: point-, -to: point-
	FILL_CIRCLE, // fill_circle -center: point-, -radius: number-

	// System functions
	SLEEP, // sleep -millisecond: number-

	// Built-in variables
	WIDTH,
	HEIGHT,

	// Built-in functions
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
	DISTANCE,

	NO_TOKEN
};

// A single token of the CW2 script
struct Token {
	std::string data;
	size_t errLine;
	TokenType type = TokenType::NO_TOKEN;
};