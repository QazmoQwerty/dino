#pragma once

#define SINGLE_LINE_COMMENT_END '\n'
#define MULTI_LINE_COMMENT_END "*/"
#define ESCAPE_CHAR '\\'

#include <vector>
#include <unordered_map>
#include <string>
#include <iostream>
#include "Token.h"
#include "OperatorsMap.h"

using std::vector;
using std::string;
using std::pair;
using std::unordered_map;

class Lexer
{
public:
	
	/*
		Sets up internal maps.
		IMPORTANT: Function MUST be called once before using any other functions of this class.
	*/
	static void setup();

	/*
		Gets a string of code (usually either interpreted or taken from a file).
		Function proccesses the code and returns a vector of Tokens.
		NOTE: Token could be of type "OperatorToken" or "LiteralToken<T>" as well as regular "Token", 
			  so make sure to check the _type variable of each token.
	*/
	static vector<Token*>& lex(string str);
private:
	static unordered_map<char, CharType> _map;
	static Token * getToken(string str, unsigned int & index, int & line);
};