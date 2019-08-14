#include "Parser.h"

int _lineNum;
int _index;

//Token * Parser::getToken(unsigned int line, unsigned int index)
//{
//	if(line >= _tokens.size() || index >= (*_tokens[line]).size())
//		return NULL;
//	return (*_tokens[line])[index];
//}
//
//Token * Parser::nextToken()
//{
//	if (_line >= _tokens.size())
//		return NULL;
//	if (_index >= (*_tokens[_line]).size())
//		_line++;
//	return getToken(_line, _index++);
//}
//
//Token * Parser::nextToken(OperatorType expected)
//{
//	if (_line >= _tokens.size())
//		return NULL;
//	if (_index >= (*_tokens[_line]).size())
//		_line++;
//	Token * t = getToken(_line, _index);
//	if (t->_type != TT_OPERATOR || ((OperatorToken*)t)->_operator._type != expected)
//		return NULL;
//	_index++;
//	return t;
//}

Token * Parser::getToken(unsigned int index)
{
	if(index >= _tokens.size())
		return NULL;
	return _tokens[index];
}

Token * Parser::nextToken(OperatorType expected)
{
	Token * token = peekToken();
	if (token->_type == TT_OPERATOR && ((OperatorToken*)token)->_operator._type == expected) 
	{
		_index++;
		return token;
	}
	std::cout << expected << " was expected, " << token->_data << "found instead" << std::endl;
	return NULL;
}

AST::Node * Parser::parse(int lastPrecedence)
{
	AST::Node* left = nud(nextToken());
	while (peekToken() && precedence(peekToken()) > lastPrecedence)
		left = led(left, nextToken());
	return left;
}

/*
	Returns the relevant operator precedence if token is an operator, otherwise returns 0.
*/
int Parser::precedence(Token * token)
{
	return (token->_type == TT_OPERATOR) ? ((OperatorToken*)token)->_operator._precedence : 0;
}

AST::Node * Parser::nud(Token * token)
{
	if (token->_type == TT_IDENTIFIER)
	{
		AST::Identificator varId;
		varId.name = token->_data;
		return new AST::Variable(varId);
	}
	if (token->_type == TT_LITERAL)
	{
		switch (((LiteralToken<int>*)token)->_literalType)
		{
			case (LT_BOOLEAN): return new AST::Boolean(((LiteralToken<bool>*)token)->_value);
			case (LT_INTEGER): return new AST::Integer(((LiteralToken<int>*)token)->_value);
			case (LT_STRING): return new AST::String(((LiteralToken<string>*)token)->_value);
			case (LT_CHARACTER): return new AST::Character(((LiteralToken<char>*)token)->_value);
			case (LT_FRACTION): return new AST::Fraction(((LiteralToken<float>*)token)->_value);
			case (LT_NULL): return new AST::Null();
			default: return NULL;	// TODO - proper error system
		}
	}
	if (token->_type == TT_OPERATOR && OperatorsMap::isUnary(((OperatorToken*)token)->_operator._type))
	{
		auto ot = ((OperatorToken*)token);
		if (ot->_operator._type == OT_PARENTHESIS_OPEN)
		{
			AST::Node* inner = parse();
			nextToken(OT_PARENTHESIS_CLOSE);
			return inner;
		}
		if (ot->_operator._type == OT_CURLY_BRACES_OPEN)
		{
			// TODO - hande statement blocks (and arrays).
			AST::Node* inner = parse();
			nextToken(OT_CURLY_BRACES_CLOSE);
			return inner;
		}

		auto op = new AST::UnaryOperation();
		op->setOperator(ot->_operator);
		int prec = ot->_operator._precedence;
		if (ot->_operator._associativity == RIGHT_TO_LEFT) prec--;
		try { op->setExpression((AST::Expression*)parse(prec)); }
		catch (exception) { throw "Could not convert from Node* to Expression*"; }	// Should be a DinoException in the future
		return op;
		
	}
	return NULL;	// Error
}

AST::Node * Parser::led(AST::Node * left, Token * token)
{
	if (token->_type == TT_OPERATOR && OperatorsMap::isBinary(((OperatorToken*)token)->_operator._type))
	{
		auto ot = ((OperatorToken*)token);
		
		
		
		if (ot->_operator._type == OT_CURLY_BRACES_OPEN)
		{
			// TODO - hande statement blocks (and arrays).
			AST::Node* inner = parse();
			nextToken(OT_CURLY_BRACES_CLOSE);
			return inner;
		}

		auto op = new AST::BinaryOperation();
		op->setOperator(ot->_operator);
		int prec = ot->_operator._precedence;
		if (ot->_operator._associativity == RIGHT_TO_LEFT) prec--;
		if (ot->_operator._type == OT_PARENTHESIS_OPEN || ot->_operator._type == OT_SQUARE_BRACKETS_OPEN)
			prec = 0;
		try { op->setLeft((AST::Expression*)left); }
		catch (exception) { throw "Could not convert from Node* to Expression*"; }	// Should be a DinoException in the future
		try { op->setRight((AST::Expression*)parse(prec)); }
		catch (exception) { throw "Could not convert from Node* to Expression*"; }	// Should be a DinoException in the future

		if (ot->_operator._type == OT_PARENTHESIS_OPEN)
			nextToken(OT_PARENTHESIS_CLOSE);
		else if (ot->_operator._type == OT_SQUARE_BRACKETS_OPEN)
			nextToken(OT_SQUARE_BRACKETS_CLOSE);
		return op;
	}
	throw "led could not find an option";
	return NULL;	// Error
}
