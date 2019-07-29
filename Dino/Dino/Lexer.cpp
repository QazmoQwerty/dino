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
		for (unsigned int i = 0; i < c.first.length(); i++)
			_dict[c.first[i]] = CT_OPERATOR;
}

vector<Token*>& Lexer::lex(string str)
{
	vector<Token*> *tokens = new vector<Token*>();
	int line = 1;
	unsigned int index = 0;
	while (index < str.length())
	{
		tokens->push_back(getToken(str, index, line));
		if (tokens->back()->_type == TT_NEWLINE)
			line++;
	}
	return *tokens;
}

Token * Lexer::getToken(string str, unsigned int & index, int line)
{
	Token* token = new struct Token;
	char curr = str[index];
	token->_data = curr;
	index++;
	switch (_dict[curr])
	{
		case CT_WHITESPACE:
		{
			token->_type = TT_WHITESPACE;
			token->_line = line;
			break;
		}

		case CT_LINE_BREAK:
		{
			token->_type = TT_LINE_BREAK;
			token->_line = line;
			break;
		}

		case CT_NEWLINE:
		{
			token->_type = TT_NEWLINE;
			token->_line = line;
			break;
		}

		case CT_DIGIT:
		{
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
				auto fractionToken = createFractionLiteralToken(token->_data, line);
				delete token;
				token = fractionToken;
			}
			else
			{
				auto intToken = createIntegerLiteralToken(token->_data, line);
				std::cout << intToken->_line << std::endl;
				delete token;
				token = intToken;
			}
			break;
		}

		case CT_LETTER:
		{
			while (index < str.length())
			{
				if (_dict[str[index]] == CT_DIGIT || _dict[str[index]] == CT_LETTER)
					token->_data += str[index];
				else break;
				index++;
			}
			token->_type = TT_IDENTIFIER;
			token->_line = line;
			break;
		}

		case CT_OPERATOR:
		{
			while (index < str.length() && _dict[str[index]] == CT_OPERATOR)
			{
				string newOp = token->_data + str[index];
				if (OperatorsMap::getOperators().count(newOp))
				{
					token->_data = newOp;
					index++;
				}
				else break;
			}
			OperatorToken * temp = new struct OperatorToken;
			temp->_data = token->_data;
			temp->_type = TT_OPERATOR;
			temp->_operatorType = OperatorsMap::getOperators()[temp->_data];
			temp->_line = line;

			if (temp->_operatorType == OT_SINGLE_QUOTE || temp->_operatorType == OT_DOUBLE_QUOTE)	// Character
			{
				char c = temp->_data[0];
				temp->_data = "";
				while (index < str.length() && str[index] != c)
				{
					temp->_data += str[index];
					index++;
				}
				index++;

				delete token;
				token = (temp->_operatorType == OT_SINGLE_QUOTE) ?
					createCharacterLiteralToken(temp->_data, line) :
					token = createStringLiteralToken(temp->_data, line);
				delete temp;
			}
			else
			{
				delete token;
				token = temp;
			}

			
			break;
			
			// TODO - deal with comments (multi line AND single line).
		}

		case CT_UNKNOWN:
			break;
	}
	return token;
}

pair<const string, OperatorType> Lexer::getOperatorByDefinition(OperatorType operatorType)
{
	for (auto t : OperatorsMap::getOperators())
		if (t.second == operatorType)
			return t;
	return pair<const string, OperatorType>("", OT_UNKNOWN);
}