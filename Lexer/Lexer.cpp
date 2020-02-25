#include "Lexer.h"

unordered_map<unicode_char, CharType, UnicodeHasherFunction> Lexer::_map;

#define UTF8(a) unicode_char(a)

/*
	Sets up values in _map.
	IMPORTANT: Function MUST be called once before using any other functions of this class.
*/
void Lexer::setup()
{
	for (auto c : OperatorsMap::getOperators())
		for (unsigned int i = 0; i < c.first.length(); i++)
			_map[c.first[i]] = CT_OPERATOR;
}

/*
	Gets a unicode_char and returns corresponding CharType.
	CT_WHITESPACE	:	' ', '\t', '\r'
	CT_NEWLINE		:	'\n'
	CT_LINE_BREAK	:	'|'
	CT_OPERATOR		:	operators defined in OperatorsMap
	CT_DIGIT		:	any unicode character from category Nd
	CT_LETTER		:	any unicode character from categories Li, Lm, Lt, Lu, Lo
*/
CharType Lexer::getCharType(unicode_char c)
{
	// TODO - use u_isalpha and u_isdigit from ICU library for all unicode letters and digits
	auto cp = c.getValue();
	switch (cp) {
		case (' '): case ('\t'): case('\r'): 
				return CT_WHITESPACE;
		case ('\n'):	return CT_NEWLINE;
		case('|'):		return CT_LINE_BREAK;
		case('_'):		return CT_LETTER;
	}

	if (('a' <= cp && cp <= 'z') || ('A' <= cp && cp <= 'Z'))
		return CT_LETTER;
	if ('0' <= cp && cp <= '9')
		return CT_DIGIT;
	return _map[c];
}


/*
	Gets a string of code (usually either interpreted or taken from a file).
	Function proccesses the code and returns a vector of Tokens.
	NOTE: Token could be of type "OperatorToken" or "LiteralToken<T>" as well as regular "Token",
			so make sure to check the _type variable of each token.
*/
vector<Token*>& Lexer::lex(unicode_string str)
{
	vector<Token*> *tokens = new vector<Token*>();
	int line = 1;
	unsigned int index = 0;
	while (index < str.length()) 
		tokens->push_back(getToken(str, index, line));
	return *tokens;
}

/*
	Gets a string of code, an index, and the current line number.
	Function creates and returns a Token based on the inputted string (starting from the index).
	NOTE: "index" and "line" parameters are passed by reference, and are 
		  changed internally, no need to increment them outside of this function.
*/
Token * Lexer::getToken(unicode_string str, unsigned int & index, int & line)
{
	Token* token = new struct Token;
	unicode_char curr = str[index];
	token->_data = curr;
	index++;
	switch (getCharType(curr))
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
				if (getCharType(str[index]) == CT_DIGIT)
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
				if (getCharType(str[index]) == CT_DIGIT || getCharType(str[index]) == CT_LETTER)
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
			else if (token->_data == "null")
			{
				Token * nullToken = createNullLiteralToken(line);
				delete token;
				token = nullToken;
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
			while (index < str.length() && getCharType(str[index]) == CT_OPERATOR)
			{
				unicode_string newOp = token->_data;
				newOp += str[index];
				if (OperatorsMap::getOperators().count(token->_data) == 0 || OperatorsMap::getOperators().count(newOp) != 0)
				{
					token->_data = newOp;
					index++;	
				}
				else break;
			}
			OperatorToken * temp = new struct OperatorToken;
			temp->_data = token->_data;
			temp->_type = TT_OPERATOR;
			temp->_operator = OperatorsMap::getOperators().find(temp->_data)->second;
			temp->_line = line;

			if (temp->_operator._type == OT_SINGLE_QUOTE || temp->_operator._type == OT_DOUBLE_QUOTE)	// Character
			{
				unicode_char c = temp->_data[0];
				temp->_data = "";

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
				if (temp->_operator._type == OT_SINGLE_QUOTE)
					token = createCharacterLiteralToken(temp->_data, line);
				else token = createStringLiteralToken(temp->_data, line);
				delete temp;
			}
			else
			{
				if (temp->_operator._type == OT_SINGLE_LINE_COMMENT)
				{
					temp->_data = "";
					while (index < str.length() && str[index] != SINGLE_LINE_COMMENT_END)
						index++;
					index++;	// Can probably find a more elegant solution
					temp->_type = TT_SINGLE_LINE_COMMENT;
					line++;
				}
				if (temp->_operator._type == OT_MULTI_LINE_COMMENT_OPEN)
				{
					temp->_data = "";
					while (index + 1 < str.length() && !(str[index] == MULTI_LINE_COMMENT_END_1 && str[index + 1] == MULTI_LINE_COMMENT_END_2))
					{
						if (str[index] == '\n')
							line++;
						index++;
					}
					index += 2;
					temp->_type = TT_MULTI_LINE_COMMENT;
				}
				delete token;
				token = temp;
			}
			break;
		}

		case CT_UNKNOWN:
			throw DinoException("internal lexer error", EXT_LEXER, line);
	}
	return token;
}
