#pragma once

#include <iostream>
#include <string>
#include "../Utils/TypeEnums.h"
#include "../Utils/Unicode/Utf8Handler.h"

#define ESCAPE_CHAR '\\'
#define LEFT_TO_RIGHT 1
#define RIGHT_TO_LEFT 2

using std::string;

/* 
	Basic information about the position of a token or node 
*/
typedef struct PositionInfo 
{
	int line;
	int startPos;
	int endPos;
	string file;
} PositionInfo;

/*
	Basic token struct.
*/
struct Token 
{
	TokenType _type;
	unicode_string _data;
	PositionInfo _pos;
};

/*
	Struct representing a dino operator.
	Information about operators comes from 'OperatorsMap'
*/
typedef struct Operator
{
	OperatorType _type;
	unicode_string _str;
	unsigned char _associativity;	// LEFT_TO_RIGHT, RIGHT_TO_LEFT, or NONE

	int _binaryPrecedence;
	int _prefixPrecedence;
	int _postfixPrecedence;
} Operator;

/*
	Struct that represents operator tokens.
	NOTE: Token::_type MUST be "TT_OPERATOR".
*/
struct OperatorToken : public Token
{
	Operator _operator;
};


/*
	Struct that represents literal tokens.
	NOTES: 
	1) _type MUST be "TT_LITERAL"
	2) _literalType must correlate to the type of _value:
		LT_BOOLEAN	  :	  bool
		LT_INTEGER	  :	  int
		LT_FRACTION	  :	  float
		LT_CHARACTER  :	  unicode_char
		LT_STRING	  :	  std::string
*/
template <class T>
struct LiteralToken : public Token
{
	LiteralType _literalType;
	T _value;
};

/*
	Function gets a Token and prints it based on its type.
	Tokens values are printed based on _data.
*/
void printToken(Token* token);

/*
	Function gets a Token of type LiteralToken and prints it based on _literalType.
	Tokens values are printed based on _data.
*/
void printLiteralToken(Token * token);

/*
	Function gets a Token of type LiteralToken and prints it based on _literalType.
	Tokens values are printed based on _value - this means that special characters
	such as '\n' will be shown by their value rather than how they were inputed.
*/
void printLiteralTokenByValue(Token * token);

/*
	Function gets the second character of a string representing a special
	character, and assuming that the first character was an escape character,
	returns the corresponding special character as a single character string.
	If not special character is found, will return the original character (as a string).
	Note: only a select few of ASCII special characters have been included.
*/
string getSpecialCharConstant(char secondChar);

/*
	Similar to getSpecialCharConstant(char), except this function also gets the
	garunteed escape character in the input string.
	Inefficient function, use getSpecialCharConstant(char) instead.
*/
string getSpecialCharConstant(string value);

/*
	Gets an input string and the current line number.
	Function creates and returns a LiteralToken with type LT_STRING based on the input.
	TODO: String literals with escaped characters (eg. '\n') in them.
*/
LiteralToken<string> * createStringLiteralToken(unicode_string data, PositionInfo pos);

/*
	Gets an input string and the current line number.
	Function creates and returns a LiteralToken with type LT_CHARACTER based on the input.
	NOTE: if input is not a valid character an exception will be thrown.
*/
LiteralToken<unicode_char> * createCharacterLiteralToken(unicode_string data, PositionInfo pos);

/*
	Gets an input string and the current line number.
	Function creates and returns a LiteralToken with type LT_FRACTION based on the input.
	NOTE: if input is not a valid fraction an exception will be thrown.
*/
LiteralToken<float> * createFractionLiteralToken(unicode_string data, PositionInfo pos);

/*
	Gets an input string and the current line number.
	Function creates and returns a LiteralToken with type LT_INTEGER based on the input.
	NOTE: if input is not a valid integer an exception will be thrown.
*/
LiteralToken<int> * createIntegerLiteralToken(unicode_string data, PositionInfo pos);

/*
	Gets an input string and the current line number.
	Function creates and returns a LiteralToken with type LT_BOOLEAN based on the input.
	NOTE: if input is not "false" or "true" an exception will be thrown.
*/
LiteralToken<bool> * createBooleanLiteralToken(unicode_string data, PositionInfo pos);

/*
	Gets an input string and the current line number.
	Function creates and returns a LiteralToken with type LT_NULL based on the input.
*/
LiteralToken<bool> * createNullLiteralToken(PositionInfo pos);