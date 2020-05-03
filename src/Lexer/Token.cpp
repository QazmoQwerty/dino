#include "Token.h"
#include "../Utils//ErrorReporter/ErrorReporter.h"

#define GET_PATH(f) (f ? f->getFullPath() : "")

/*
	Function gets a Token and prints it based on its type.
	Tokens values are printed based on _data.
*/
void printToken(Token * token)
{
	switch (token->_type)
	{
		case (TT_WHITESPACE):
			std::cout << GET_PATH(token->_pos.file) << ":" << token->_pos.line << ":" << token->_pos.startPos << ":" << token->_pos.endPos << " - [WHITESPACE]" << std::endl;
			break;
		case (TT_NEWLINE):
			std::cout << GET_PATH(token->_pos.file) << ":" << token->_pos.line << ":" << token->_pos.startPos << ":" << token->_pos.endPos << " - [NEWLINE]" << std::endl;
			break;
		case (TT_OPERATOR):
			std::cout << GET_PATH(token->_pos.file) << ":" << token->_pos.line << ":" << token->_pos.startPos << ":" << token->_pos.endPos << " - [OPERATOR: " << token->_data.to_string() << "]" << std::endl;
			break;
		case (TT_LINE_BREAK):
			std::cout << GET_PATH(token->_pos.file) << ":" << token->_pos.line << ":" << token->_pos.startPos << ":" << token->_pos.endPos << " - [LINE_BREAK: " << token->_data.to_string() << "]" << std::endl;
			break;
		case (TT_IDENTIFIER):
			std::cout << GET_PATH(token->_pos.file) << ":" << token->_pos.line << ":" << token->_pos.startPos << ":" << token->_pos.endPos << " - [IDENTIFIER: " << token->_data.to_string() << "]" << std::endl;
			break;
		case (TT_LITERAL):
			printLiteralToken(token);
			break;
		case (TT_SINGLE_LINE_COMMENT):
			std::cout << GET_PATH(token->_pos.file) << ":" << token->_pos.line << ":" << token->_pos.startPos << ":" << token->_pos.endPos << " - [SINGLE_LINE_COMMENT]" << std::endl;
			break;
		case (TT_MULTI_LINE_COMMENT):
			std::cout << GET_PATH(token->_pos.file) << ":" << token->_pos.line << ":" << token->_pos.startPos << ":" << token->_pos.endPos << " - [MULTI_LINE_COMMENT]" << std::endl;
			break;
		case (TT_UNKNOWN):
			std::cout << GET_PATH(token->_pos.file) << ":" << token->_pos.line << ":" << token->_pos.startPos << ":" << token->_pos.endPos << " - [UNKNOWN]" << std::endl;
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
			std::cout << GET_PATH(token->_pos.file) << ":" << token->_pos.line << ":" << token->_pos.startPos << ":" << token->_pos.endPos << " - [BOOLEAN: " << token->_data.to_string() << "]" << std::endl;
			break;
		case (LT_INTEGER):
			std::cout << GET_PATH(token->_pos.file) << ":" << token->_pos.line << ":" << token->_pos.startPos << ":" << token->_pos.endPos << " - [INTEGER: " << token->_data.to_string() << "]" << std::endl;
			break;
		case (LT_STRING):
			std::cout << GET_PATH(token->_pos.file) << ":" << token->_pos.line << ":" << token->_pos.startPos << ":" << token->_pos.endPos << " - [STRING: " << token->_data.to_string() << "]" << std::endl;
			break;
		case (LT_CHARACTER):
			std::cout << GET_PATH(token->_pos.file) << ":" << token->_pos.line << ":" << token->_pos.startPos << ":" << token->_pos.endPos << " - [CHARACTER: " << token->_data.to_string() << "]" << std::endl;
			break;
		case (LT_FRACTION):
			std::cout << GET_PATH(token->_pos.file) << ":" << token->_pos.line << ":" << token->_pos.startPos << ":" << token->_pos.endPos << " - [FRACTION: " << token->_data.to_string() << "]" << std::endl;
			break;
		case (LT_NULL):
			std::cout << GET_PATH(token->_pos.file) << ":" << token->_pos.line << ":" << token->_pos.startPos << ":" << token->_pos.endPos << " - [NULL]" << std::endl;
			break;
		default: break;
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
			std::cout << GET_PATH(token->_pos.file) << ":" << token->_pos.line << ":" << token->_pos.startPos << ":" << token->_pos.endPos << " - [BOOLEAN: " << ((LiteralToken<bool>*)token)->_value << "]" << std::endl;
			break;
		case (LT_INTEGER):
			std::cout << GET_PATH(token->_pos.file) << ":" << token->_pos.line << ":" << token->_pos.startPos << ":" << token->_pos.endPos << " - [INTEGER: " << ((LiteralToken<int>*)token)->_value << "]" << std::endl;
			break;
		case (LT_STRING):
			std::cout << GET_PATH(token->_pos.file) << ":" << token->_pos.line << ":" << token->_pos.startPos << ":" << token->_pos.endPos << " - [STRING: \"" << ((LiteralToken<string>*)token)->_value << "\"]" << std::endl;
			break;
		case (LT_CHARACTER):
			std::cout << GET_PATH(token->_pos.file) << ":" << token->_pos.line << ":" << token->_pos.startPos << ":" << token->_pos.endPos << " - [CHARACTER: " << ((LiteralToken<unicode_char>*)token)->_data.to_string() << "]" << std::endl;
			break;
		case (LT_FRACTION):
			std::cout << GET_PATH(token->_pos.file) << ":" << token->_pos.line << ":" << token->_pos.startPos << ":" << token->_pos.endPos << " - [FRACTION: " << ((LiteralToken<float>*)token)->_value << "]" << std::endl;
			break;
		case (LT_NULL):
			std::cout << GET_PATH(token->_pos.file) << ":" << token->_pos.line << ":" << token->_pos.startPos << ":" << token->_pos.endPos << " - [NULL]" << std::endl;
			break;
		default: break;
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
LiteralToken<string> * createStringLiteralToken(unicode_string value, PositionInfo pos)
{
	LiteralToken<string> * token = new struct LiteralToken<string>;
	token->_data = unicode_string("\"");
	token->_data += value;
	token->_data += *new unicode_string("\"");
	token->_pos = pos;
	token->_literalType = LT_STRING;
	token->_type = TT_LITERAL;
	/*for (unsigned int i = 1; i < value.length(); i++) // TODO - special characters
		if (value[i - 1] == "\\")
			value.replace(i - 1, 2, getSpecialCharConstant(value[i]));*/
	token->_value = value.to_string();	// TODO - unicode string literals
	return token;
}

/*
	Gets an input string and the current line number.
	Function creates and returns a LiteralToken with type LT_CHARACTER based on the input.
	NOTE: if input is not a valid character an exception will be thrown.
*/
LiteralToken<unicode_char> * createCharacterLiteralToken(unicode_string value, PositionInfo pos)	
{
	char val = 'f';
	if (value.length() == 0)
		throw ErrorReporter::report("Empty char constant", ERR_LEXER, pos);
	if (value.length() > 1) 
	{
		if (value.length() == 2 && value[0].getValue() == ESCAPE_CHAR) 
		{
			if (value[1].getValue() == 't')
				val = '\t';
				// value = unicode_string("\t");
			else if (value[1].getValue() == 'r')
				val = '\r';
				// value = unicode_string("\r");
			else if (value[1].getValue() == 'n')
				val = '\n';
				// value = unicode_string("\n");
			else if (value[1].getValue() == '0')
				val = '\0';
				// value = unicode_string("\0");
			else if (value[1].getValue() == '\\')
				val = '\\';
				// value = unicode_string("\\");
			else if (value[1].getValue() == '\'')
				val = '\'';
				// value = unicode_string("'");
			else throw ErrorReporter::report("Too many characters in character constant", ERR_LEXER, pos);
		}
		else throw ErrorReporter::report("Too many characters in character constant", ERR_LEXER, pos);
	}
	struct LiteralToken<unicode_char> * token = new struct LiteralToken<unicode_char>;
	token->_data = unicode_string("'");
	token->_data += value;
	token->_data += "'";
	token->_pos = pos;
	token->_literalType = LT_CHARACTER;
	token->_type = TT_LITERAL;
	if (val == 'f')
		token->_value = value[0];
	else token->_value = val;
	return token;
}

/*
	Gets an input string and the current line number.
	Function creates and returns a LiteralToken with type LT_FRACTION based on the input.
	NOTE: if input is not a valid fraction an exception will be thrown.
*/
LiteralToken<float> * createFractionLiteralToken(unicode_string data, PositionInfo pos)
{
	LiteralToken<float> * token = new struct LiteralToken<float>;
	token->_data = data;
	token->_pos = pos;
	token->_type = TT_LITERAL;
	token->_literalType = LT_FRACTION;
	try { token->_value = stof(data.to_string()); }
	catch (exception) { throw ErrorReporter::report("Invalid fraction literal", ERR_LEXER, pos); }
	return token;
}

/*
	Gets an input string and the current line number.
	Function creates and returns a LiteralToken with type LT_INTEGER based on the input.
	NOTE: if input is not a valid integer an exception will be thrown.
*/
LiteralToken<int> * createIntegerLiteralToken(unicode_string data, PositionInfo pos)
{
	LiteralToken<int> * token = new struct LiteralToken<int>;
	token->_data = data;
	token->_pos = pos;
	token->_type = TT_LITERAL;
	token->_literalType = LT_INTEGER;
	try { token->_value = stoi(data.to_string()); }
	catch (exception) { throw ErrorReporter::report("Invalid integer literal", ERR_LEXER, pos); }
	return token;
}

/*
	Gets an input string and the current line number.
	Function creates and returns a LiteralToken with type LT_BOOLEAN based on the input.
	NOTE: if input is not "false" or "true" an exception will be thrown.
*/
LiteralToken<bool> * createBooleanLiteralToken(unicode_string data, PositionInfo pos)
{
	LiteralToken<bool> * temp = new struct LiteralToken<bool>;
	temp->_data = data;
	temp->_pos = pos;
	temp->_type = TT_LITERAL;
	temp->_literalType = LT_BOOLEAN;
	if (data == "true")
		temp->_value = true;
	else if (data == "false")
		temp->_value = false;
	else throw ErrorReporter::report("Invalid boolean literal", ERR_LEXER, pos);
	return temp;
}

LiteralToken<bool>* createNullLiteralToken(PositionInfo pos)
{
	LiteralToken<bool> * temp = new struct LiteralToken<bool>;
	temp->_data = "null";
	temp->_pos = pos;
	temp->_type = TT_LITERAL;
	temp->_literalType = LT_NULL;
	return temp;
}
