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
	if (index >= _tokens.size())
		return NULL;
	return _tokens[index];
}

Token * Parser::nextToken(OperatorType expected)
{
	Token * token = peekToken();
	if (isOperator(token, expected))
	{
		_index++;
		return token;
	}
	//std::cout << expected << " was expected, " << token->_data << "found instead" << std::endl;
	return NULL;
}

AST::Block * Parser::parseBlock(OperatorType expected)
{
	auto block = new AST::Block();
	while (peekToken() && !eatOperator(expected))
	{
		if (peekToken()->_type == TT_LINE_BREAK)
			nextToken();
		auto node = parse();
		if (node)
			block->addStatement(node);
		//else std::cout << "Empty block?" << std::endl;
		if (eatOperator(expected))
			return block;
		nextToken();
	}
	return block;
}

AST::Node * Parser::parse(int lastPrecedence)
{
	AST::Node* left = nud(nextToken());

	if (left == NULL)
		return NULL;

	if (left->isStatement() && (((AST::Statement*)left)->getType() == ST_WHILE_LOOP ||
		((AST::Statement*)left)->getType() == ST_IF_THEN_ELSE))	// not the most elegant of solutions
		return left;

	/*if (left->isExpression() && ((AST::Expression*)left)->getType() == TT_IDENTIFIER && peekToken()->_type == TT_IDENTIFIER)
	{ 
		auto decl = new AST::VariableDeclaration();

		decl->setType(*(AST::Identificator*)left);
	}*/

	while (peekToken()->_type != TT_LINE_BREAK && !isOperator(peekToken(), OT_EOF) && !isOperator(peekToken(), OT_CURLY_BRACES_OPEN) && precedence(peekToken()) > lastPrecedence)
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

		if (peekToken()->_type == TT_IDENTIFIER)
		{
			auto node = new AST::VariableDeclaration();
			node->setType(varId);
			varId.name = peekToken()->_data;
			node->setVarId(varId);
			nextToken();

			while (peekToken()->_type == TT_IDENTIFIER)
			{
				node->addModifier(node->getVarType());
				node->setType(node->getVarId());
				varId.name = peekToken()->_data;
				node->setVarId(varId);
				nextToken();
			}

			return node;
		}
		
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

		if (ot->_operator._type == OT_WHILE)
		{
			AST::WhileLoop * node = new AST::WhileLoop();
			AST::Node* inner = parse();
			if (inner->isExpression())
				node->setCondition((AST::Expression*)inner);
			else throw "could not convert from Node* to Expression*";
			
			while (peekToken()->_type == TT_LINE_BREAK)
				nextToken();

			if (isOperator(peekToken(), OT_CURLY_BRACES_OPEN))
			{
				nextToken();
				node->setStatement(parseBlock(OT_CURLY_BRACES_CLOSE));
			}
			else throw "could not parse while loop";
			return node;
		}

		if (ot->_operator._type == OT_IF)
		{
			AST::IfThenElse * node = new AST::IfThenElse();
			AST::Node* inner = parse();
			if (inner->isExpression())
				node->setCondition((AST::Expression*)inner);
			else throw "could not convert from Node* to Expression*";

			while (peekToken()->_type == TT_LINE_BREAK)
				nextToken();

			if (isOperator(peekToken(), OT_CURLY_BRACES_OPEN))
			{
				nextToken();
				node->setThenBranch(parseBlock(OT_CURLY_BRACES_CLOSE));
			}
			else throw "could not parse while loop";

			bool b = false;
			while (peekToken()->_type == TT_LINE_BREAK)
			{
				b = true;
				nextToken();
			}
			if (eatOperator(OT_ELSE))
			{
				auto p = parse();
				if (p->isStatement())
					node->setElseBranch((AST::Statement*)p);
				else throw "else branch must be a statement!";
			}
			else if (b) _index--;

			return node;
		}

		if (ot->_operator._type == OT_PARENTHESIS_OPEN)
		{
			AST::Node* inner = parse();
			nextToken(OT_PARENTHESIS_CLOSE);
			return inner;
		}
		if (ot->_operator._type == OT_CURLY_BRACES_OPEN)
		{
			AST::Node* inner = parseBlock(OT_CURLY_BRACES_CLOSE);
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

		if (ot->_operator._type == OT_PARENTHESIS_OPEN)
		{
			
			if (left->isExpression() && ((AST::Expression*)left)->getType() == ET_VARIABLE)
			{
				auto funcCall = new AST::FunctionCall();
				auto varId = ((AST::Variable*)left)->getVarId();	// this line needs to be split into two parts for some wierd reason
				funcCall->setFunctionId(varId);
				while (!isOperator(peekToken(), OT_PARENTHESIS_CLOSE))
				{
					auto exp = parse(10);
					if (!exp->isExpression())
						throw "Could not convert from Node* to Expression*";
					funcCall->addParameter((AST::Expression*)exp);
					if (isOperator(peekToken(), OT_COMMA))
						nextToken();
				}
				nextToken(OT_PARENTHESIS_CLOSE);
				return funcCall;
			}
			else throw "Expression preceding parenthesis of apparent call must be a variable id";
		}

		auto op = new AST::BinaryOperation();
		op->setOperator(ot->_operator);
		int prec = ot->_operator._precedence;
		if (ot->_operator._associativity == RIGHT_TO_LEFT) prec--;
		try { op->setLeft((AST::Expression*)left); }
		catch (exception) { throw "Could not convert from Node* to Expression*"; }	// Should be a DinoException in the future
		try { op->setRight((AST::Expression*)parse(prec)); }
		catch (exception) { throw "Could not convert from Node* to Expression*"; }	// Should be a DinoException in the future

		if (ot->_operator._type == OT_SQUARE_BRACKETS_OPEN)
			nextToken(OT_SQUARE_BRACKETS_CLOSE);

		return op;
	}
	throw "led could not find an option";
	return NULL;	// Error
}
