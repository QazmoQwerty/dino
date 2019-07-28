#pragma once
#include <iostream>
#include "TypeEnums.h"

struct Token 
{
	TokenType _type;
	std::string _data;
	int _line;
} ;

struct OperatorToken : public Token
{
	OperatorType _operatorType;
} OperatorToken;

template <class T>
struct LiteralToken : public Token
{
	LiteralType _literalType;
	T _value;
};