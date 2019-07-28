#include "Lexer.h"

unordered_map<char, CharType> Lexer::_dict;

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
		for(unsigned int i = 0; i < c.first.length(); i++)	
			_dict[c.first[i]] = CT_OPERATOR;
}

vector<Token*>& Lexer::lex(string str)
{
	//Token* currentToken;

	vector<Token*> *tokens = new vector<Token*>();
	for (unsigned int i = 0; i < str.length(); i++)
	{
		char curr = str[i];
	}
	return *tokens;
}

Token * Lexer::getToken(string str, int index)
{
	Token* t = new struct Token;
	char curr = str[index];
	t->_data = curr;

	switch (_dict[curr])
	{
		case CT_WHITESPACE:
			break;
	}


	return nullptr;
}
