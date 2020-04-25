/*
    Utility functions to figure out operator precedence.
*/
#include "Parser.h"

/*
	Returns the relevant operator precedence from the selected category(s) if token is an operator, otherwise returns 0.
*/
int Parser::precedence(Token * token, int category)
{
	if (token->_type == TT_IDENTIFIER)
		return 135;
	if (token->_type != TT_OPERATOR)
		return 0;
	auto op = ((OperatorToken*)token)->_operator;
	switch (category) {
		case(BINARY):	return op._binaryPrecedence;
		case(PREFIX):	return op._prefixPrecedence;
		case(POSTFIX):	return op._postfixPrecedence;
		case(BINARY | POSTFIX):	return op._binaryPrecedence != NONE ? op._binaryPrecedence : op._postfixPrecedence;
		default:		return 0;
	}
	
}

int Parser::leftPrecedence(OperatorToken * token, int category)
{
	int prec = precedence(token, category);
	if (token->_operator._associativity == RIGHT_TO_LEFT) prec--;
	return prec;
}