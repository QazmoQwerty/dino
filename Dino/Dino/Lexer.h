#pragma once
#include <vector>
#include "Token.h"

class Lexer
{
public:
	std::vector<Token*>& lex(std::string);
private:

};