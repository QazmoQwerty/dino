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
		for (uint i = 0; i < c.first.length(); i++)
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
vector<Token*> Lexer::lex(unicode_string &str, SourceFile* file)
{
	vector<Token*> tokens;
	uint index = 0, line = 1, pos = 0;
	while (index < str.length()) 
		if (auto tok = getToken(str, index, line, pos, file, tokens))
			tokens.push_back(tok);

	auto eof = new OperatorToken;
	eof->_data = "EOF";
	eof->_type = TT_OPERATOR;
	if (tokens.size() == 0)
		throw ErrorReporter::report("empty program", ErrorReporter::GENERAL_ERROR, ErrorReporter::POS_NONE);
	eof->_pos = tokens.back()->_pos;
	eof->_operator = { OT_EOF, unicode_string("EOF"), NON_ASSCOCIATIVE, 0, 0, 0 };
	tokens.push_back(eof);
	return tokens;
}

/*
	Gets a string of code, an index, and the current line number.
	Function creates and returns a Token based on the inputted string (starting from the index).
	NOTE: "index" and "line" parameters are passed by reference, and are 
		  changed internally, no need to increment them outside of this function.
*/
Token * Lexer::getToken(unicode_string &str, uint & index, uint & line, uint & pos, SourceFile* file, vector<Token*>& tokens)
{
	static bool ignoreLineBreak = false;

	Token* token = new struct Token;
	unicode_char curr = str[index++];
	token->_data = curr;
	token->_pos.line = line;
	token->_pos.file = file;
	token->_pos.startPos = pos++;
	token->_pos.endPos = pos;
	switch (getCharType(curr))
	{
		case CT_WHITESPACE:
		{
			delete token;
			return NULL;
		}

		case CT_LINE_BREAK:
		{
			token->_type = TT_LINE_BREAK;
			break;
		}

		case CT_NEWLINE:
		{
			token->_type = TT_NEWLINE;
			line++;
			pos = 0;
			break;
		}

		case CT_DIGIT:
		{
			bool isFraction = false;
			while (index < str.length())
			{
				if (getCharType(str[index]) == CT_DIGIT)
					token->_data += str[index];
				else if (str[index] == '.' && !isFraction && index + 1 < str.length() && getCharType(str[index+1]) == CT_DIGIT)
				{
					isFraction = true;
					token->_data += '.';
				}
				else break;
				index++;
				pos++;
			}
			if (isFraction)
			{
				auto fractionToken = createFractionLiteralToken(token->_data, token->_pos);
				delete token;
				token = fractionToken;
			}
			else
			{
				auto intToken = createIntegerLiteralToken(token->_data, token->_pos);
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
				pos++;
			}
			
			if (token->_data == "false" || token->_data == "true")
			{
				Token * booleanToken = createBooleanLiteralToken(token->_data, token->_pos);
				delete token;
				token = booleanToken;
			}
			else if (token->_data == "null")
			{
				Token * nullToken = createNullLiteralToken(token->_pos);
				delete token;
				token = nullToken;
			}
			else if (OperatorsMap::getWordOperators().count(token->_data))
			{
				auto ot = new OperatorToken;
				ot->_data = token->_data;
				ot->_pos = token->_pos;
				ot->_type = TT_OPERATOR;
				ot->_operator = OperatorsMap::getWordOperators().find(ot->_data)->second;
				delete token;
				token = ot;
			}
			else token->_type = TT_IDENTIFIER;
			token->_pos.endPos = pos;
			
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
					pos++;
				}
				else break;
			}
			OperatorToken * temp = new struct OperatorToken;
			temp->_data = token->_data;
			temp->_type = TT_OPERATOR;
			temp->_operator = OperatorsMap::getOperators().find(temp->_data)->second;
			temp->_pos = token->_pos;

			if (temp->_operator._type == OT_SINGLE_QUOTE || temp->_operator._type == OT_DOUBLE_QUOTE)	// Character
			{
				unicode_char c = temp->_data[0];
				temp->_data = "";

				while (index < str.length())
				{
					if (str[index] == c)
					{
						uint count = 0;
						while (count < index && str[index - count - 1] == ESCAPE_CHAR)
							count++;
						if (count % 2 == 0)	// Check if number of escape characters before string/char closer is even.
							break;
					}
					else if (str[index] == '\n')
					{
						line++;
						pos = 0;
					}
					temp->_data += str[index];
					index++;
					pos++;
				}
				index++;
				pos++;

				delete token;
				if (temp->_operator._type == OT_SINGLE_QUOTE)
					token = createCharacterLiteralToken(temp->_data, temp->_pos);
				else token = createStringLiteralToken(temp->_data, temp->_pos);
				delete temp;
			}
			else
			{
				if (temp->_operator._type == OT_SINGLE_LINE_COMMENT)
				{
					temp->_data = "";
					while (index < str.length() && str[index] != SINGLE_LINE_COMMENT_END)
						{ index++; pos++; }
					index++;	// Can probably find a more elegant solution
					temp->_type = TT_SINGLE_LINE_COMMENT;
					line++;
					pos = 0;
				}
				else if (temp->_operator._type == OT_MULTI_LINE_COMMENT_OPEN)
				{
					while (index + 1 < str.length() && !(str[index] == MULTI_LINE_COMMENT_END_1 && str[index + 1] == MULTI_LINE_COMMENT_END_2))
					{
						if (str[index] == '\n')
						{
							line++;
							pos = 0;
						}
						else pos++;
						index++;
					}
					index += 2;
					pos += 2;
					delete temp;
					delete token;
					return NULL;
				}
				delete token;
				token = temp;
			}
			break;
		}

		case CT_UNKNOWN:
			throw ErrorReporter::report("Unknown character", ErrorReporter::GENERAL_ERROR, token->_pos);
	}
	if (pos != 0)
		token->_pos.endPos = pos;

	if (token->_type == TT_SINGLE_LINE_COMMENT || token->_type == TT_NEWLINE)
	{
		bool b = false;
		if (tokens.size() != 0)
		{
			if (tokens.back()->_type == TT_IDENTIFIER || tokens.back()->_type == TT_LITERAL)
				b = true;
			else if (tokens.back()->_type == TT_OPERATOR) {
				switch (((OperatorToken*)tokens.back())->_operator._type)
				{
					case OT_RETURN: case OT_BREAK: case OT_CONTINUE: case OT_EXTERN: case OT_INCREMENT: case OT_DECREMENT: 
					case OT_CURLY_BRACES_CLOSE: case OT_PARENTHESIS_CLOSE: case OT_SQUARE_BRACKETS_CLOSE: case OT_AT: case OT_GET: case OT_SET:
						b = true;
					default: break;
				}
			}
		}
		if (b)
		{
			token->_type = TT_LINE_BREAK;
			token->_data = "\n";
		}
		else {
			delete token;
			return NULL;
		}
	}

	if (token->_type == TT_LINE_BREAK && (ignoreLineBreak || tokens.size() == 0 || tokens.back()->_type == TT_LINE_BREAK))
	{
		delete token;
		return NULL;
	}

	if (token->_type == TT_OPERATOR)
	{
		switch (((OperatorToken*)token)->_operator._type) 
		{
			case OT_IGNORE_LINEBREAK:
				ignoreLineBreak = true;
				if (tokens.size() != 0 && tokens.back()->_type == TT_LINE_BREAK)
				{
					delete tokens.back();
					tokens.pop_back();
				}
				delete token;
				return NULL;
			case OT_THEN: case OT_LOGICAL_AND: case OT_LOGICAL_OR: case OT_GET: case OT_SET:
				if (tokens.size() != 0 && tokens.back()->_type == TT_LINE_BREAK)
				{
					delete tokens.back();
					tokens.pop_back();
				}
				ignoreLineBreak = false;
				break;
			case OT_LOGICAL_NOT:
			{
				if (tokens.size() != 0 && tokens.back()->_type == TT_OPERATOR && ((OperatorToken*)tokens.back())->_operator._type == OT_IS)
				{
					delete tokens.back();
					tokens.pop_back();
					((OperatorToken*)token)->_operator = OperatorsMap::getOperatorByDefinition(OT_IS_NOT).second;
					token->_data = "is not";
				}
				ignoreLineBreak = false;
				break;
			}
			default: 
				ignoreLineBreak = false; 
				break;
		}
	}
	else ignoreLineBreak = false;

	return token;
}
