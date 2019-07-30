#pragma once

#include <iostream>
#include <string>
#include "Exceptions.h"
#include "TypeEnums.h"

#define ESCAPE_CHAR '\\'

using std::string;

struct Token 
{
	TokenType _type;
	string _data;
	int _line;
};

struct OperatorToken : public Token
{
	OperatorType _operatorType;
};

template <class T>
struct LiteralToken : public Token
{
	LiteralType _literalType;
	T _value;
};

void printToken(Token* token);
void printLiteralToken(Token * token);
void printLiteralTokenByValue(Token * token);
string getSpecialCharConstant(string value);
string getSpecialCharConstant(char secondChar);

LiteralToken<string> * createStringLiteralToken(string data, int line);
LiteralToken<char> * createCharacterLiteralToken(string data, int line);
LiteralToken<float> * createFractionLiteralToken(string data, int line);
LiteralToken<int> * createIntegerLiteralToken(string data, int line);
LiteralToken<bool> * createBooleanLiteralToken(string data, int line);
