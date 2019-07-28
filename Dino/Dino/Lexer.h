#pragma once
#include <vector>
#include "Token.h"

class Lexer
{
public:
	static std::vector<Token*>& lex(std::string);
private:

};