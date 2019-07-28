#pragma once
#include <vector>
#include <unordered_map>
#include "Token.h"

class Lexer
{
public:
	static void setup();
	static std::vector<Token*>& lex(std::string str);
private:
	static std::unordered_map<char, CharType> _dict;
	static std::unordered_map<char, OperatorType> operators;
};