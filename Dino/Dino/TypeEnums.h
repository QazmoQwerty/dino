#pragma once

enum TokenType
{
	NEWLINE,
	OPERATOR,
	LINE_BREAK,
	IDENTIFIER,
	LITERAL,
	UNKNOWN,
};

enum LiteralType
{
	BOOLEAN,
	NULL_VALUE,	// TODO - find a better name for NULL (can't use NULL because it's already taken)
	INTEGER,
	FRACTION,
	CHARACTER,
	STRING,
	UNKNOWN,
};

enum OperatorType
{
	// TODO
	UNKNOWN,
};

enum CharType
{
	WHITESPACE,
	LINE_BREAK,
	NEWLINE,
	LETTER,
	DIGIT,
	OPERATOR,
	UNKNOWN,
};