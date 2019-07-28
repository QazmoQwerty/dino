#pragma once

#include <vector>
#include <unordered_map>
#include "Token.h"
#include "OperatorsMap.h"

using std::vector;
using std::string;
using std::unordered_map;

class Lexer
{
public:
	static void setup();
	static std::vector<Token*>& lex(std::string str);
private:
	static std::unordered_map<char, CharType> _dict;
};