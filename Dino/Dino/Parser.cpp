#include "Parser.h"

int _lineNum;
int _index;

AST::Node * Parser::Parse(unsigned int lastPrecedence)
{
	AST::Node* left = nud(NULL);
	while (true);
	return nullptr;
}

/*
	Returns the relevant operator precedence if token is an operator, otherwise returns 0.
*/
unsigned int Parser::calcPrecedence(Token * token)
{
	return (token->_type == TT_OPERATOR) ? ((OperatorToken*)token)->_operator._precedence : 0;
}

AST::Node * Parser::nud(Token * token)
{
	return nullptr;
}

AST::Node * Parser::led(Token * left, Token * current)
{
	return nullptr;
}
