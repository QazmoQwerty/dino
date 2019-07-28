#pragma once

enum TokenType
{
	TT_WHITESPACE,
	TT_NEWLINE,
	TT_OPERATOR,
	TT_LINE_BREAK,
	TT_IDENTIFIER,
	TT_LITERAL,
	TT_UNKNOWN,
};

enum LiteralType
{
	LT_BOOLEAN,
	LT_NULL,
	LT_INTEGER,
	LT_FRACTION,
	LT_CHARACTER,
	LT_STRING,
	LT_UNKNOWN,
};

enum OperatorType
{
	// TODO
	OT_UNKNOWN,
};

enum CharType
{
	CT_WHITESPACE,
	CT_LINE_BREAK,
	CT_NEWLINE,
	CT_LETTER,
	CT_DIGIT,
	CT_OPERATOR,
	CT_UNKNOWN,
};