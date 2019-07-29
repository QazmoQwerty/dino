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
	static void setup();
	static vector<Token*>& lex(string str);
private:
	static unordered_map<char, CharType> _dict;
	static Token * getToken(string str, unsigned int & index, int & line);
	static pair<const string, OperatorType> getOperatorByDefinition(OperatorType operatorType);
};