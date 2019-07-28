#include "Lexer.h"

void Lexer::setup()
{
	_dict[' '] = _dict['\t'] = _dict['\r'] = CT_WHITESPACE;
	_dict['\n'] = CT_NEWLINE;
	_dict[';'] = CT_LINE_BREAK;
	_dict['_'] = CT_LETTER;
	for (char c = 'a'; c <= 'z'; c++)
		_dict[c] = CT_LETTER;
	for (char c = 'A'; c <= 'Z'; c++)
		_dict[c] = CT_LETTER;
	for (char c = '0'; c <= '9'; c++)
		_dict[c] = CT_DIGIT;
	for (auto c : OperatorsMap::getOperators())
		for(int i = 0; i < c.first.length(); i++)
			_dict[c.first[i]] = CT_OPERATOR;
}

std::vector<Token*>& Lexer::lex(std::string str)
{
	Token* currentToken;
	
	std::vector<Token*> tokens;
	for (int i = 0; i < str.length(); i++)
	{
		char curr = str[i];
	}
}
