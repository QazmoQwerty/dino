#include "Token.h"
#include "OperatorsMap.h"

void printToken(Token * token)
{
	switch (token->_type)
	{
		case(TT_WHITESPACE):
			std::cout << "line " << token->_line << " - [WHITESPACE: " << token->_data << "]" << std::endl;
			break;
		case(TT_NEWLINE):
			std::cout << "line " << token->_line << " - [NEWLINE]" << std::endl;
			break;
		case(TT_OPERATOR):
			std::cout << "line " << token->_line << " - [OPERATOR (opId " << ((OperatorToken*)token)->_operatorType << "): "<< token->_data << "]" << std::endl;
			break;
		case(TT_LINE_BREAK):
			std::cout << "line " << token->_line << " - [LINE_BREAK: " << token->_data << "]" << std::endl;
			break;
		case(TT_IDENTIFIER):
			std::cout << "line " << token->_line << " - [IDENTIFIER: " << token->_data << "]" << std::endl;
			break;
		case(TT_LITERAL):
			printLiteralToken(token);
			break;
		case(TT_UNKNOWN):
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
			std::cout << "line " << token->_line << " - [STRING: \"" << token->_data << "\"]" << std::endl;
			break;
		case (LT_CHARACTER):
			std::cout << "line " << token->_line << " - [CHARACTER: '" << token->_data << "']" << std::endl;
			break;
		case (LT_FRACTION):
			std::cout << "line " << token->_line << " - [FRACTION: " << token->_data << "]" << std::endl;
			break;
		case (LT_NULL):
			std::cout << "line " << token->_line << " - [NULL]" << std::endl;
			break;
	}
}
