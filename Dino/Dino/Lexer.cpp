#include "Lexer.h"

void Lexer::setup()
{
	_dict[' '] = _dict['\t'] = _dict['\r'] = WHITESPACE;
	_dict['\n'] = NEWLINE;
	_dict[';'] = LINE_BREAK;
	_dict['_'] = LETTER;
	for (char c = 'a'; c <= 'z'; c++)
		_dict[c] = LETTER;
	for (char c = 'A'; c <= 'Z'; c++)
		_dict[c] = LETTER;
	for (char c = '0'; c <= '9'; c++)
		_dict[c] = DIGIT;
	for (auto c : operators)
		_dict[c.first] = OPERATOR;
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

std::vector<char> Lexer::getOperatorsList()
{
	return std::vector<char>();
}
