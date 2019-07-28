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

Token * Lexer::getToken(string str, int & index, int line)
{
	Token* token = new struct Token;
	char curr = str[index];
	token->_data = curr;
	index++;
	switch (_dict[curr])
	{
		case CT_WHITESPACE:
			token->_type = TT_WHITESPACE;
			break;

		case CT_LINE_BREAK:
			token->_type = TT_LINE_BREAK;
			break;

		case CT_NEWLINE:
			token->_type = TT_NEWLINE;
			break;

		case CT_DIGIT:
			bool isFraction = false;
			while (index < str.length())
			{
				if (_dict[str[index]] == CT_DIGIT)
					token->_data += str[index];
				else if (str[index] == '.' && !isFraction)
				{
					isFraction = true;
					token->_data += '.';
				}
				else break;
				index++;
			}
			if (isFraction)
			{
				LiteralToken<float> * temp = new struct LiteralToken<float>;
				temp->_data = token->_data;
				temp->_type = TT_LITERAL;
				temp->_literalType = LT_FRACTION;
				temp->_value = stof(temp->_data);	// TODO - exception handling
			}
			else
			{
				LiteralToken<int> * temp = new struct LiteralToken<int>;
				temp->_data = token->_data;
				temp->_type = TT_LITERAL;
				temp->_literalType = LT_INTEGER;
				temp->_value = stoi(temp->_data);	// TODO - exception handling
			}


			break;

		case CT_LETTER:
			break;

		case CT_OPERATOR:
			break;

		case CT_UNKNOWN:
			break;
	}


	token->_line = line;
	return nullptr;
}
