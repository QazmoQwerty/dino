#pragma once

#define SINGLE_LINE_COMMENT_END '\n'
#define MULTI_LINE_COMMENT_END_1 '*'
#define MULTI_LINE_COMMENT_END_2 '/'
#define ESCAPE_CHAR '\\'

#include <vector>
#include <unordered_map>
#include <string>
#include <iostream>
#include "Token.h"
#include "../Utils/OperatorsMap/OperatorsMap.h"
#include "../Utils/ErrorReporter/ErrorReporter.h"

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
	static vector<Token*>& lex(unicode_string str, string fileName);
private:
	static unordered_map<unicode_char, CharType, UnicodeHasherFunction> _map;
	static Token * getToken(unicode_string str, unsigned int & index, unsigned int & line, unsigned int & pos, string fileName);

	static CharType getCharType(unicode_char c);
};