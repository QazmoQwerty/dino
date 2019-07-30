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
		/*if (tokens->back()->_type == TT_NEWLINE)
			line++;*/
	}
	return *tokens;
}

Token * Lexer::getToken(string str, unsigned int & index, int & line)
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
			line++;
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
			
			if (token->_data == "false" || token->_data == "true")
			{
				Token * booleanToken = createBooleanLiteralToken(token->_data, line);
				delete token;
				token = booleanToken;
			}
			else
			{
				token->_type = TT_IDENTIFIER;
				token->_line = line;
			}
			
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

				// Temporary fix - escape character can't be escaped currently.
				/*while (index < str.length() && !(str[index] == c && str[index - 1] != ESCAPE_CHAR))	
				{
					temp->_data += str[index];
					index++;
				}*/

				while (index < str.length())
				{
					if (str[index] == c)
					{
						unsigned int count = 0;
						while (count < index && str[index - count - 1] == ESCAPE_CHAR)
							count++;
						if (count % 2 == 0)	// Check if number of escape characters before string/char closer is even.
							break;
					}
					else if (str[index] == '\n')
						line++;
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
				if (temp->_operatorType == OT_SINGLE_LINE_COMMENT)
				{
					temp->_data = "";
					while (index < str.length() && str[index] != SINGLE_LINE_COMMENT_END)
						index++;
					index++;	// Can probably find a more elegant solution
					temp->_type = TT_COMMENT;
					line++;
				}
				else if (temp->_operatorType == OT_MULTI_LINE_COMMENT_OPEN)
				{
					temp->_data = "";
					while (index + 1 < str.length() && string() + str[index] + str[index + 1] != MULTI_LINE_COMMENT_END)
					{
						if (str[index] == '\n')
							line++;
						index++;
					}
					index += 2;
					temp->_type = TT_COMMENT;
				}
				delete token;
				token = temp;
			}
			break;
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