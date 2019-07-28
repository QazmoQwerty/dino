#pragma once

#include <iostream>
#include <string>
#include "TypeEnums.h"

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