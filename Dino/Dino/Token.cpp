#include "Token.h"
#include "OperatorsMap.h"

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
			printLiteralTokenByValue(token);
			break;
		case (TT_COMMENT):
			std::cout << "line " << token->_line << " - [COMMENT]" << std::endl;
			break;
		case (TT_UNKNOWN):
			std::cout << "line " << token->_line << " - [UNKNOWN]" << std::endl;
			break;
	}
}

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

LiteralToken<string> * createStringLiteralToken(string value, int line)
{
	LiteralToken<string> * token = new struct LiteralToken<string>;
	token->_data = '"' + value + '"';
	token->_line = line;
	token->_literalType = LT_STRING;
	token->_type = TT_LITERAL;
	token->_value = value;
	return token;
}

LiteralToken<char> * createCharacterLiteralToken(string value, int line)
{
	// TODO - special characters (like '\n')
	if (value.length() != 1)
		return NULL;	// TODO - exception handling
	LiteralToken<char> * token = new struct LiteralToken<char>;
	token->_data = '\'' + value + '\'';
	token->_line = line;
	token->_literalType = LT_CHARACTER;
	token->_type = TT_LITERAL;
	token->_value = value[0];
	return token;
}

LiteralToken<float> * createFractionLiteralToken(string data, int line)
{
	LiteralToken<float> * token = new struct LiteralToken<float>;
	token->_data = data;
	token->_line = line;
	token->_type = TT_LITERAL;
	token->_literalType = LT_FRACTION;
	token->_value = stof(data);	// TODO - exception handling
	return token;
}

LiteralToken<int> * createIntegerLiteralToken(string data, int line)
{
	LiteralToken<int> * token = new struct LiteralToken<int>;
	token->_data = data;
	token->_line = line;
	token->_type = TT_LITERAL;
	token->_literalType = LT_INTEGER;
	token->_value = stoi(data);	// TODO - exception handling
	return token;
}

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
	// TODO - exception handling
	return temp;
}