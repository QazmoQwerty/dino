#include "Parser.h"

int _lineNum;
int _index;

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
	return NULL;
}

AST::StatementBlock * Parser::parseBlock(OperatorType expected)
{
	auto block = new AST::StatementBlock();
	while (peekToken() && !eatOperator(expected))
	{
		if (peekToken()->_type == TT_LINE_BREAK)
			nextToken();
		if (eatOperator(expected))
			return block;
		auto node = parse();
		if (node) 
		{
			if (node->isStatement())
				block->addStatement(dynamic_cast<AST::Statement*>(node));
			else throw "unused expression";
		}

		if (eatOperator(expected))
			return block;
		nextToken();
	}
	return block;
}

AST::Node * Parser::parse(int lastPrecedence)
{
	Token* tok = nextToken();

	AST::Node* left = std(tok);
	if (left) return left;

	left = nud(tok);
	if (left == NULL) return NULL;

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

AST::Node * Parser::std(Token * token)
{
	if (token->_type == TT_OPERATOR && OperatorsMap::isUnary(((OperatorToken*)token)->_operator._type))
	{
		auto ot = ((OperatorToken*)token);

		if (ot->_operator._type == OT_WHILE)
		{
			AST::WhileLoop * node = new AST::WhileLoop();
			AST::Node* inner = parse();
			if (inner->isExpression())
				node->setCondition(dynamic_cast<AST::Expression*>(inner));
			else throw "could not convert from Node* to Expression*";

			while (peekToken()->_type == TT_LINE_BREAK)
				nextToken();

			if (eatOperator(OT_CURLY_BRACES_OPEN))
				node->setStatement(parseBlock(OT_CURLY_BRACES_CLOSE));
			else throw "could not parse while loop";

			return node;
		}

		if (ot->_operator._type == OT_FOR)
		{
			AST::StatementBlock *node = new AST::StatementBlock();

			if (!eatLineBreak())
			{
				do
				{
					AST::Node *declaration = parse(OperatorsMap::getOperatorByDefinition(OT_COMMA).second._precedence);
					if (declaration->isStatement()/* && declaration->getNodeId()*/)	// why is the commented out code here?
						node->addStatement(dynamic_cast<AST::Statement*>(declaration));
					else throw "for's decleration statement failed";
				} while (eatOperator(OT_COMMA));
			}
			AST::WhileLoop * whileNode = new AST::WhileLoop();

			if (!eatLineBreak())
				throw "miss part 2 of for";

			AST::Node* inner = parse();
			if (inner->isExpression())
				whileNode->setCondition(dynamic_cast<AST::Expression*>(inner));
			else throw "could not convert from Node* to Expression*";

			if (!eatLineBreak())
				throw "miss 3rd part of for loop";

			vector<AST::Node *> increments;
			do
			{
				AST::Node *increment = parse(OperatorsMap::getOperatorByDefinition(OT_COMMA).second._precedence);
				if (increment->getNodeId())
					increments.push_back(increment);
				else throw "for's decleration statement failed";
			} while (eatOperator(OT_COMMA));

			while (peekToken()->_type == TT_LINE_BREAK)
				nextToken();

			AST::StatementBlock *innerBlock = new AST::StatementBlock();
			if (eatOperator(OT_CURLY_BRACES_OPEN))
				innerBlock = parseBlock(OT_CURLY_BRACES_CLOSE);
			else throw "could not parse while loop";

			for (AST::Node* increment : increments)
				if (increment->isStatement())
					innerBlock->addStatement(dynamic_cast<AST::Statement*>(increment));
				else throw "TODO - error";

			whileNode->setStatement(innerBlock);

			node->addStatement(whileNode);
			return node;
		}

		if (ot->_operator._type == OT_DO)
		{
			AST::DoWhileLoop * node = new AST::DoWhileLoop();

			while (peekToken()->_type == TT_LINE_BREAK)
				nextToken();

			if (eatOperator(OT_CURLY_BRACES_OPEN))
				node->setStatement(parseBlock(OT_CURLY_BRACES_CLOSE));
			else throw "could not do parse while loop";

			if (!eatOperator(OT_WHILE))
				throw "Missing 'while' in do-while statement.";

			AST::Node* inner = parse();
			if (inner->isExpression())
				node->setCondition(dynamic_cast<AST::Expression*>(inner));
			else throw "could not convert from Node* to Expression*";

			return node;
		}

		if (ot->_operator._type == OT_IF)
		{
			AST::IfThenElse * node = new AST::IfThenElse();
			AST::Node* inner = parse();

			if (inner && inner->isExpression())
				node->setCondition(dynamic_cast<AST::Expression*>(inner));
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
				AST::Node *p = NULL;
				if (isOperator(peekToken(), OT_IF))
				{
					p = parse();
				}
				else if (eatOperator(OT_CURLY_BRACES_OPEN))
				{
					p = parseBlock(OT_CURLY_BRACES_CLOSE);
				}
				if (p && p->isStatement())
					node->setElseBranch(dynamic_cast<AST::Statement*>(p));
				else throw "else branch must be a statement!";
			}
			else if (b) _index--;

			return node;
		}
	}
	return NULL;
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
			if (isOperator(peekToken(), OT_PARENTHESIS_OPEN))	// function declaration
			{
				node->addModifier({ "func" });
				auto func = new AST::Function();

				while (!eatOperator(OT_PARENTHESIS_CLOSE))
				{
					auto param = parse(10);
					//if (!exp->isExpression())
					//	throw "Could not convert from Node* to Expression*";
					if (param == NULL)
						break;
					if (!param->isStatement() || (dynamic_cast<AST::Statement*>(param))->getStatementType() != ST_VARIABLE_DECLARATION)
						throw "could not convert to VariableDeclaration";
					func->addParameter(dynamic_cast<AST::VariableDeclaration*>(param));
					peekToken();
					eatOperator(OT_COMMA);
				}
				nextToken(OT_PARENTHESIS_CLOSE);
				if (!eatOperator(OT_CURLY_BRACES_OPEN))
					throw "function has no body";
				func->setContent(parseBlock(OT_CURLY_BRACES_CLOSE));

				auto bo = new AST::BinaryOperation();
				bo->setOperator(OperatorsMap::getOperatorByDefinition(OT_ASSIGN_EQUAL).second);
				bo->setLeft((AST::Expression*)node); //eew
				bo->setRight(func);
				return bo;
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
		if (ot->_operator._type == OT_INCREMENT || ot->_operator._type == OT_DECREMENT)
		{
			auto op = new AST::Increment();
			op->setOperator(ot->_operator);
			int prec = ot->_operator._precedence;
			if (ot->_operator._associativity == RIGHT_TO_LEFT) prec--;
			try { op->setExpression(dynamic_cast<AST::Expression*>(parse(prec))); }
			catch (exception) { throw "Could not convert from Node* to Expression*"; }	// Should be a DinoException in the future
			return op;
		}

		auto op = new AST::UnaryOperation();
		op->setOperator(ot->_operator);
		int prec = ot->_operator._precedence;
		if (ot->_operator._associativity == RIGHT_TO_LEFT) prec--;
		try { op->setExpression(dynamic_cast<AST::Expression*>(parse(prec))); }
		catch (exception) { throw "Could not convert from Node* to Expression*"; }	// Should be a DinoException in the future
		return op;
		
	}
	return NULL; // Error
}

AST::Node * Parser::led(AST::Node * left, Token * token)
{
	if (token->_type == TT_OPERATOR && OperatorsMap::isBinary(((OperatorToken*)token)->_operator._type))
	{
		auto ot = ((OperatorToken*)token);

		if (ot->_operator._type == OT_PARENTHESIS_OPEN)
		{	
			if (left->isExpression() && dynamic_cast<AST::Expression*>(left)->getExpressionType() == ET_VARIABLE)
			{
				auto funcCall = new AST::FunctionCall();
				auto varId = dynamic_cast<AST::Variable*>(left)->getVarId();	// this line needs to be split into two parts for some wierd reason
				funcCall->setFunctionId(varId);
				while (!eatOperator(OT_PARENTHESIS_CLOSE))
				{
					auto exp = parse(10);
					if (!exp->isExpression())
						throw "Could not convert from Node* to Expression*";
					funcCall->addParameter(dynamic_cast<AST::Expression*>(exp));
					if (isOperator(peekToken(), OT_COMMA))
						nextToken();
				}
				return funcCall;
			}
			else throw "Expression preceding parenthesis of apparent call must be a variable id";
		}
		if (OperatorsMap::isAssignment(ot->_operator._type))
		{
			auto op = new AST::Assignment();
			op->setOperator(ot->_operator);
			int prec = ot->_operator._precedence;
			if (ot->_operator._associativity == RIGHT_TO_LEFT) prec--;
			try { op->setLeft(dynamic_cast<AST::Expression*>(left)); }
			catch (exception) { throw "Could not convert from Node* to Expression*"; }	// Should be a DinoException in the future
			try { op->setRight(dynamic_cast<AST::Expression*>(parse(prec))); }
			catch (exception) { throw "Could not convert from Node* to Expression*"; }	// Should be a DinoException in the future
			return op;
		}

		auto op = new AST::BinaryOperation();
		op->setOperator(ot->_operator);
		int prec = ot->_operator._precedence;
		if (ot->_operator._associativity == RIGHT_TO_LEFT) prec--;
		try { op->setLeft(dynamic_cast<AST::Expression*>(left)); }
		catch (exception) { throw "Could not convert from Node* to Expression*"; }	// Should be a DinoException in the future
		try { op->setRight(dynamic_cast<AST::Expression*>(parse(prec))); }
		catch (exception) { throw "Could not convert from Node* to Expression*"; }	// Should be a DinoException in the future

		if (ot->_operator._type == OT_SQUARE_BRACKETS_OPEN)
			nextToken(OT_SQUARE_BRACKETS_CLOSE); // TODO - should be "parseBlock"

		return op;
	}
	throw "led could not find an option";
	return NULL;	// Error
}
