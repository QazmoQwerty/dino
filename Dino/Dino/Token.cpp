#include "Token.h"
#include "OperatorsMap.h"

/*
	Function gets a Token and prints it based on its type.
	Tokens values are printed based on _data.
*/
void printToken(Token * token)
{
	switch (token->_type)
	{
		case (TT_WHITESPACE):
			std::cout << "line " << token->_line << " - [WHITESPACE]" << std::endl;
			break;
		case (TT_NEWLINE):
			std::cout << "line " << token->_line << " - [NEWLINE]" << std::endl;
			break;
		case (TT_OPERATOR):
			std::cout << "line " << token->_line << " - [OPERATOR: "<< token->_data << "]" << std::endl;
			break;
		case (TT_LINE_BREAK):
			std::cout << "line " << token->_line << " - [LINE_BREAK: " << token->_data << "]" << std::endl;
			break;
		case (TT_IDENTIFIER):
			std::cout << "line " << token->_line << " - [IDENTIFIER: " << token->_data << "]" << std::endl;
			break;
		case (TT_LITERAL):
			printLiteralToken(token);
			break;
		case (TT_SINGLE_LINE_COMMENT):
			std::cout << "line " << token->_line << " - [SINGLE_LINE_COMMENT]" << std::endl;
			break;
		case (TT_MULTI_LINE_COMMENT):
			std::cout << "line " << token->_line << " - [MULTI_LINE_COMMENT]" << std::endl;
			break;
		case (TT_UNKNOWN):
			std::cout << "line " << token->_line << " - [UNKNOWN]" << std::endl;
			break;
	}
}

/*
	Function gets a Token of type LiteralToken and prints it based on _literalType.
	Tokens values are printed based on _data.
*/
void printLiteralToken(Token * token)
{
	switch (((LiteralToken<int>*)token)->_literalType)
	{
		case (LT_BOOLEAN):
			std::cout << "line " << token->_line << " - [BOOLEAN: " << token->_data << "]" << std::endl;
			break;
		case (LT_INTEGER):
			std::cout << "line " << token->_line << " - [INTEGER: " << token->_data << "]" << std::endl;
			break;
		case (LT_STRING):
			std::cout << "line " << token->_line << " - [STRING: " << token->_data << "]" << std::endl;
			break;
		case (LT_CHARACTER):
			std::cout << "line " << token->_line << " - [CHARACTER: " << token->_data << "]" << std::endl;
			break;
		case (LT_FRACTION):
			std::cout << "line " << token->_line << " - [FRACTION: " << token->_data << "]" << std::endl;
			break;
		case (LT_NULL):
			std::cout << "line " << token->_line << " - [NULL]" << std::endl;
			break;
	}
}

/*
	Function gets a Token of type LiteralToken and prints it based on _literalType.
	Tokens values are printed based on _value - this means that special characters 
	such as '\n' will be shown by their value rather than how they were inputed.
*/
void printLiteralTokenByValue(Token * token)
{
	switch (((LiteralToken<int>*)token)->_literalType)
	{
		case (LT_BOOLEAN):
			std::cout << "line " << token->_line << " - [BOOLEAN: " << ((LiteralToken<bool>*)token)->_value << "]" << std::endl;
			break;
		case (LT_INTEGER):
			std::cout << "line " << token->_line << " - [INTEGER: " << ((LiteralToken<int>*)token)->_value << "]" << std::endl;
			break;
		case (LT_STRING):
			std::cout << "line " << token->_line << " - [STRING: \"" << ((LiteralToken<string>*)token)->_value << "\"]" << std::endl;
			break;
		case (LT_CHARACTER):
			std::cout << "line " << token->_line << " - [CHARACTER: '" << ((LiteralToken<char>*)token)->_value << "']" << std::endl;
			break;
		case (LT_FRACTION):
			std::cout << "line " << token->_line << " - [FRACTION: " << ((LiteralToken<float>*)token)->_value << "]" << std::endl;
			break;
		case (LT_NULL):
			std::cout << "line " << token->_line << " - [NULL]" << std::endl;
			break;
	}
}

/*
	Function gets the second character of a string representing a special 
	character, and assuming that the first character was an escape character, 
	returns the corresponding special character as a single character string.
	If not special character is found, will return the original character (as a string).
	Note: only a select few of ASCII special characters have been included.
*/
string getSpecialCharConstant(char secondChar)
{
	switch (secondChar)
	{
		case 'b':
			return "\b";
		case 'n':
			return "\n";
		case 't':
			return "\t";
		case 'v':
			return "\v";
		case '"':
			return "\"";
		case '\'':
			return "'";
		case '\\':
			return "\\";
		default:
			return string() + secondChar;	// no special character found
	}
}

/*
	Similar to getSpecialCharConstant(char), except this function also gets the 
	garunteed escape character in the input string.
	Inefficient function, use getSpecialCharConstant(char) instead.
*/
string getSpecialCharConstant(string value)
{
	// Note: not all ASCII special characters have been included.
	if (value == "\\b")		// Backspace
		return "\b";
	if (value == "\\n")		// Newline
		return "\n";
	if (value == "\\t")		// Horizontal Tab
		return "\t";
	if (value == "\\v")		// Vertical Tab
		return "\v";
	if (value == "\\\\")	// Backslash
		return "\\";
	if (value == "\\'")		// Single Quotation Mark
		return "'";
	if (value == "\\\"")	// Single Quotation Mark
		return "\"";
	return value;
}

/*
	Gets an input string and the current line number.
	Function creates and returns a LiteralToken with type LT_STRING based on the input.
*/
LiteralToken<string> * createStringLiteralToken(string value, int line)
{
	LiteralToken<string> * token = new struct LiteralToken<string>;
	token->_data = '"' + value + '"';
	token->_line = line;
	token->_literalType = LT_STRING;
	token->_type = TT_LITERAL;
	for (unsigned int i = 1; i < value.length(); i++)
		if (value[i - 1] == '\\')
			value.replace(i - 1, 2, getSpecialCharConstant(value[i]));
	token->_value = value;
	return token;
}

/*
	Gets an input string and the current line number.
	Function creates and returns a LiteralToken with type LT_CHARACTER based on the input.
	NOTE: if input is not a valid character an exception will be thrown.
*/
LiteralToken<char> * createCharacterLiteralToken(string value, int line)	
{
	if (value.length() == 0)
		throw DinoException("Empty char constant", EXT_LEXER, line);
	string tempData = value;
	if (value[0] == '\\' && value.length() >= 2)
		value = getSpecialCharConstant(value[1]);
	if (value.length() != 1)
		throw DinoException("Too many characters in character constant", EXT_LEXER, line);
	LiteralToken<char> * token = new struct LiteralToken<char>;
	token->_data = '\'' + tempData + '\'';
	token->_line = line;
	token->_literalType = LT_CHARACTER;
	token->_type = TT_LITERAL;
	token->_value = value[0];
	return token;
}

/*
	Gets an input string and the current line number.
	Function creates and returns a LiteralToken with type LT_FRACTION based on the input.
	NOTE: if input is not a valid fraction an exception will be thrown.
*/
LiteralToken<float> * createFractionLiteralToken(string data, int line)
{
	LiteralToken<float> * token = new struct LiteralToken<float>;
	token->_data = data;
	token->_line = line;
	token->_type = TT_LITERAL;
	token->_literalType = LT_FRACTION;
	try { token->_value = stof(data); }
	catch (exception) { throw DinoException("Invalid fraction literal", EXT_LEXER, line); }
	return token;
}

/*
	Gets an input string and the current line number.
	Function creates and returns a LiteralToken with type LT_INTEGER based on the input.
	NOTE: if input is not a valid integer an exception will be thrown.
*/
LiteralToken<int> * createIntegerLiteralToken(string data, int line)
{
	LiteralToken<int> * token = new struct LiteralToken<int>;
	token->_data = data;
	token->_line = line;
	token->_type = TT_LITERAL;
	token->_literalType = LT_INTEGER;
	try { token->_value = stoi(data); }
	catch (exception) { throw DinoException("Invalid integer literal", EXT_LEXER, line); }
	return token;
}

/*
	Gets an input string and the current line number.
	Function creates and returns a LiteralToken with type LT_BOOLEAN based on the input.
	NOTE: if input is not "false" or "true" an exception will be thrown.
*/
LiteralToken<bool> * createBooleanLiteralToken(string data, int line)
{
	LiteralToken<bool> * temp = new struct LiteralToken<bool>;
	temp->_data = data;
	temp->_line = line;
	temp->_type = TT_LITERAL;
	temp->_literalType = LT_BOOLEAN;
	if (data == "true")
		temp->_value = true;
	else if (data == "false")
		temp->_value = false;
	else throw DinoException("Invalid boolean literal", EXT_LEXER, line);
	return temp;
}

LiteralToken<bool>* createNullLiteralToken(int line)
{
	LiteralToken<bool> * temp = new struct LiteralToken<bool>;
	temp->_data = "null";
	temp->_line = line;
	temp->_type = TT_LITERAL;
	temp->_literalType = LT_NULL;
	return temp;
}
