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

	if (isOperator(tok, OT_EOF) || isOperator(tok, OT_PARENTHESIS_CLOSE) || isOperator(tok, OT_CURLY_BRACES_OPEN))
	{
		_index--;
		return NULL;
	}

	AST::Node* left = std(tok);
	if (left) return left;

	left = nud(tok);
	if (left == NULL) return NULL;


	while (peekToken()->_type != TT_LINE_BREAK && !isOperator(peekToken(), OT_EOF) && !isOperator(peekToken(), OT_CURLY_BRACES_OPEN) &&
		(peekToken()->_type == TT_IDENTIFIER || precedence(peekToken(), BINARY | POSTFIX) > lastPrecedence))
		left = led(left, nextToken());
	return left;
}


#define fromCategory(tok, cat) precedence(tok, cat) != NONE

/*
	Returns the relevant operator precedence from the selected category(s) if token is an operator, otherwise returns NULL.
*/
int Parser::precedence(Token * token, int category)
{
	if (token->_type != TT_OPERATOR)
		return NULL;
	auto op = ((OperatorToken*)token)->_operator;
	switch (category) {
		case(BINARY):	return op._binaryPrecedence;
		case(PREFIX):	return op._prefixPrecedence;
		case(POSTFIX):	return op._postfixPrecedence;
		case(BINARY | POSTFIX):	return op._binaryPrecedence != NONE ? op._binaryPrecedence : op._postfixPrecedence;
		default:		return NULL;
	}
	
}

AST::Node * Parser::std(Token * token)
{
	if (token->_type == TT_OPERATOR && OperatorsMap::isKeyword(((OperatorToken*)token)->_operator))
	{
		auto ot = ((OperatorToken*)token);
		if (ot->_operator._type == OT_WHILE)
		{
			AST::WhileLoop * node = new AST::WhileLoop();
			AST::Node* inner = parse();
			if (inner->isExpression())
				node->setCondition(dynamic_cast<AST::Expression*>(inner));
			else throw "could not convert from Node* to Expression*";

			while (eatLineBreak());

			if (eatOperator(OT_CURLY_BRACES_OPEN))
				node->setStatement(parseBlock(OT_CURLY_BRACES_CLOSE));
			else if (eatOperator(OT_COLON))
			{
				while (eatLineBreak());
				AST::Node* n = parse();
				if (n && n->isStatement())
					node->setStatement(dynamic_cast<AST::Statement*>(n));
				else throw "inner content of while statement must be a statement!";
			}
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
					AST::Node *declaration = parse(OperatorsMap::getOperatorByDefinition(OT_COMMA).second._binaryPrecedence);
					if (declaration->isStatement())
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
				AST::Node *increment = parse(OperatorsMap::getOperatorByDefinition(OT_COMMA).second._binaryPrecedence);
				if (increment->getNodeId())
					increments.push_back(increment);
				else throw "for's decleration statement failed";
			} while (eatOperator(OT_COMMA));

			while (peekToken()->_type == TT_LINE_BREAK)
				nextToken();

			AST::StatementBlock *innerBlock = nullptr;
			if (eatOperator(OT_CURLY_BRACES_OPEN))
				innerBlock = parseBlock(OT_CURLY_BRACES_CLOSE);
			else if (eatOperator(OT_COLON))
			{
				innerBlock = new AST::StatementBlock();
				while (eatLineBreak());
				AST::Node* n = parse();
				if (n && n->isStatement())
					innerBlock->addStatement(dynamic_cast<AST::Statement*>(n));
				else throw "inner content of a for loop's statement must be a statement!";
			}
			else throw "could not parse for loop's statement";

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

			while (eatLineBreak());

			if (eatOperator(OT_CURLY_BRACES_OPEN))
				node->setStatement(parseBlock(OT_CURLY_BRACES_CLOSE));
			else if (eatOperator(OT_COLON))
			{
				while (eatLineBreak());
				AST::Node* n = parse();
				if (n && n->isStatement())
					node->setStatement(dynamic_cast<AST::Statement*>(n));
				else throw "inner content of do-while statement must be a statement!";
			}
			else throw "could not do parse do-while loop";

			while (eatLineBreak());
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

			if (eatOperator(OT_CURLY_BRACES_OPEN))
				node->setThenBranch(parseBlock(OT_CURLY_BRACES_CLOSE));
			else if (eatOperator(OT_COLON))
			{
				while (eatLineBreak());
				AST::Node* n = parse();
				if (n && n->isStatement())
					node->setThenBranch(dynamic_cast<AST::Statement*>(n));
				else throw "inner content of if statement must be a statement!";
			}
			else throw "could not then part of if statement";

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
					p = parse();
				else if (eatOperator(OT_COLON))
				{
					while (eatLineBreak());
					p = parse();
				}
				else if (eatOperator(OT_CURLY_BRACES_OPEN))
					p = parseBlock(OT_CURLY_BRACES_CLOSE);
				if (p && p->isStatement())
					node->setElseBranch(dynamic_cast<AST::Statement*>(p));
				else throw "else branch must be a statement!";
			}
			else if (b) _index--;

			return node;
		}
		if (ot->_operator._type == OT_UNLESS)
		{
			AST::IfThenElse * node = new AST::IfThenElse();
			AST::Node* inner = parse();

			if (inner && inner->isExpression())
				node->setCondition(dynamic_cast<AST::Expression*>(inner));
			else throw "could not convert from Node* to Expression*";

			while (peekToken()->_type == TT_LINE_BREAK)
				nextToken();

			if (eatOperator(OT_CURLY_BRACES_OPEN))
				node->setElseBranch(parseBlock(OT_CURLY_BRACES_CLOSE));
			else if (eatOperator(OT_COLON))
			{
				while (eatLineBreak());
				AST::Node* n = parse();
				if (n && n->isStatement())
					node->setElseBranch(dynamic_cast<AST::Statement*>(n));
				else throw "inner content of if statement must be a statement!";
			}
			else throw "could not then part of if statement";

			return node;
		}
		if (ot->_operator._type == OT_TYPE)
		{
			auto decl = new AST::TypeDeclaration();
			if (peekToken()->_type == TT_IDENTIFIER)
				decl->setName(nextToken()->_data);
			else throw "could not parse type declaration";

			if (eatOperator(OT_IS))
				do
				{
					if (peekToken()->_type != TT_IDENTIFIER)
						throw "could not parse type declaration, incorrect use of 'is' operator";
					decl->addInterface(nextToken()->_data);
				} while (eatOperator(OT_COMMA));

			while (eatLineBreak());
			if (!eatOperator(OT_CURLY_BRACES_OPEN))
				throw "could not parse type declaration";
			while (eatLineBreak());
			while (!eatOperator(OT_CURLY_BRACES_CLOSE))
			{
				auto temp = parse();
				if (!temp->isStatement())
					throw "could not parse type declaration";
				switch (dynamic_cast<AST::Statement*>(temp)->getStatementType())
				{
				case(ST_VARIABLE_DECLARATION):
					decl->addVariableDeclaration(dynamic_cast<AST::VariableDeclaration*>(temp));
					break;
				case(ST_FUNCTION_DECLARATION):	// TODO - should be AST::FunctionDeclaration (nonexistant type atm)
					decl->addFunctionDeclaration(dynamic_cast<AST::FunctionDeclaration*>(temp));
					break;
				case(ST_PROPERTY_DECLARATION):
					decl->addPropertyDeclaration(dynamic_cast<AST::PropertyDeclaration*>(temp));
					break;
				}
				while(eatLineBreak());
			}
			return decl;
		}
		if (ot->_operator._type == OT_INTERFACE)
		{
			auto decl = new AST::InterfaceDeclaration();
			if (peekToken()->_type == TT_IDENTIFIER)
				decl->setName({ nextToken()->_data });
			else throw "could not parse interface declaration";
			eatLineBreak();
			if (eatOperator(OT_IS)) 
				do 
				{
					if (peekToken()->_type != TT_IDENTIFIER)
						throw "could not parse interface declaration, incorrect use of 'is' operator";
					decl->addImplements({ nextToken()->_data });
				} while (eatOperator(OT_COMMA));

			eatLineBreak();
			if (!eatOperator(OT_CURLY_BRACES_OPEN))
				throw "could not parse interface declaration";
			eatLineBreak();
			while (!eatOperator(OT_CURLY_BRACES_CLOSE))
			{
				auto temp = parse();
				if (!temp->isStatement())
					throw "could not parse interface declaration body";

				if (dynamic_cast<AST::Statement*>(temp)->getStatementType() == ST_VARIABLE_DECLARATION)
					decl->addProperty(dynamic_cast<AST::VariableDeclaration*>(temp));
				else if (dynamic_cast<AST::Statement*>(temp)->getStatementType() == ST_FUNCTION_DECLARATION)
					decl->addFunction(dynamic_cast<AST::FunctionDeclaration*>(temp));
				else throw "body of interface declaration may contain only declarations";
				eatLineBreak();
			}
			return decl;
		}
		if (ot->_operator._type == OT_NAMESPACE)
		{
			auto decl = new AST::NamespaceDeclaration();
			if (peekToken()->_type == TT_IDENTIFIER)
				decl->setName({ nextToken()->_data });
			else throw "could now parse namespace definition";
			while (eatLineBreak());
			if (eatOperator(OT_CURLY_BRACES_OPEN))
				decl->setStatement(parseBlock(OT_CURLY_BRACES_CLOSE));
			else throw "could now parse namespace definition";
			return decl;
		}
		if (ot->_operator._type == OT_DELETE || ot->_operator._type == OT_RETURN)
		{
			auto op = new AST::UnaryOperationStatement();
			op->setOperator(ot->_operator);
			//int prec = precedence(ot);
			//if (ot->_operator._associativity == RIGHT_TO_LEFT) prec--;
			//try { op->setExpression(dynamic_cast<AST::Expression*>(parse(prec))); }
			try { op->setExpression(dynamic_cast<AST::Expression*>(parse())); }
			catch (exception) { throw "Could not convert from Node* to Expression*"; }	// Should be a DinoException in the future
			return op;
		}
	}
	return NULL;
}

AST::Node * Parser::nud(Token * token)
{
	if (token->_type == TT_IDENTIFIER)
	{
		return new AST::Variable(token->_data);
	}
	if (token->_type == TT_LITERAL)
	{
		switch (((LiteralToken<int>*)token)->_literalType)
		{
			case (LT_BOOLEAN): return new AST::Boolean(((LiteralToken<bool>*)token)->_value);
			case (LT_INTEGER): return new AST::Integer(((LiteralToken<int>*)token)->_value);
			case (LT_STRING):  return new AST::String(((LiteralToken<string>*)token)->_value);
			case (LT_CHARACTER): return new AST::Character(((LiteralToken<char>*)token)->_value);
			case (LT_FRACTION): return new AST::Fraction(((LiteralToken<float>*)token)->_value);
			case (LT_NULL): return new AST::Null();
			default: return NULL;	// TODO - proper error system
		}
	}
	if (isOperator(token, OT_PARENTHESIS_OPEN))
	{
		AST::Node* inner = parse();
		if (!eatOperator(OT_PARENTHESIS_CLOSE))
			throw "')' missing";
		// Function literal
		if (peekToken()->_type == TT_IDENTIFIER && (inner == nullptr || inner->isExpression()))
		{
			// make sure inner is a nullptr OR VariableDeclaration OR ExpressionList of VariableDeclaration
			if (inner != nullptr)
				switch (dynamic_cast<AST::Expression*>(inner)->getExpressionType())	
				{ 	
					case (ET_LIST):
						for (auto i : dynamic_cast<AST::ExpressionList*>(inner)->getExpressions())
							if (i->getExpressionType() != ET_VARIABLE_DECLARATION)
								return inner;
					case(ET_VARIABLE_DECLARATION): break;
					default: return inner;
				}
			auto func = new AST::Function();
			if (inner != nullptr)
				func->addParameter(inner);
			AST::Node* returnType = parse();
			if (!returnType->isExpression())
				throw "type specifier must be an expression";
			func->setReturnType(dynamic_cast<AST::Expression*>(returnType));

			vector<AST::VariableDeclaration*> vec;

			if (eatOperator(OT_CURLY_BRACES_OPEN))
				func->setContent(parseBlock(OT_CURLY_BRACES_CLOSE));

			else if (eatOperator(OT_COLON))
			{
				while (eatLineBreak());
				AST::Node* n = parse(10);
				if (n && n->isStatement())
				{
					auto content = new AST::StatementBlock();
					content->addStatement(dynamic_cast<AST::Statement*>(n));
					func->setContent(content);
				}
				else throw "inner content of function must be a statement!";
			}
			else throw "missing function literal body";

			while (eatLineBreak());
			return func;
		}

		return inner;
	}
	if (fromCategory(token, PREFIX))
	{
		auto ot = ((OperatorToken*)token);
		auto op = new AST::UnaryOperation();
		op->setOperator(ot->_operator);
		int prec = precedence(ot, PREFIX);
		if (ot->_operator._associativity == RIGHT_TO_LEFT) prec--;
		try { op->setExpression(dynamic_cast<AST::Expression*>(parse(prec))); }
		catch (exception) { throw "Could not convert from Node* to Expression*"; }	// Should be a DinoException in the future
		return op;
	}
	throw "nud could not find an option";
}

AST::Node * Parser::led(AST::Node * left, Token * token)
{
	// variable declaration
	if (left->isExpression() && token->_type == TT_IDENTIFIER)
	{
		auto varDecl = new AST::VariableDeclaration();
		varDecl->setType(dynamic_cast<AST::Expression*>(left));
		varDecl->setVarId(token->_data);

		// Property declaration
		if (eatOperator(OT_CURLY_BRACES_OPEN))
		{
			

			auto decl = new AST::PropertyDeclaration(varDecl);

			while (eatLineBreak());

			if (eatOperator(OT_GET)) {
				if (eatOperator(OT_CURLY_BRACES_OPEN))
					decl->setGet(parseBlock(OT_CURLY_BRACES_CLOSE));
				else if (eatOperator(OT_COLON))
				{
					while (eatLineBreak());
					AST::Node* n = parse();
					if (n && n->isStatement())
						decl->setGet(dynamic_cast<AST::Statement*>(n));
					else throw "inner content of get statement must be a statement!";
				}
				else throw "could not parse get operator";

				while (eatLineBreak());
				if (eatOperator(OT_SET)) {
					if (eatOperator(OT_CURLY_BRACES_OPEN))
						decl->setSet(parseBlock(OT_CURLY_BRACES_CLOSE));
					else if (eatOperator(OT_COLON))
					{
						while (eatLineBreak());
						AST::Node* n = parse();
						if (n && n->isStatement())
							decl->setSet(dynamic_cast<AST::Statement*>(n));
						else throw "inner content of set statement must be a statement!";
					}
					else throw "could not parse set operator";
				}
			}

			else if (eatOperator(OT_SET)) {
				if (eatOperator(OT_CURLY_BRACES_OPEN))
					decl->setSet(parseBlock(OT_CURLY_BRACES_CLOSE));
				else if (eatOperator(OT_COLON))
				{
					while (eatLineBreak());
					AST::Node* n = parse();
					if (n && n->isStatement())
						decl->setSet(dynamic_cast<AST::Statement*>(n));
					else throw "inner content of set statement must be a statement!";
				}
				else throw "could not parse set operator";

				while (eatLineBreak());
				if (eatOperator(OT_GET)) {
					if (eatOperator(OT_CURLY_BRACES_OPEN))
						decl->setGet(parseBlock(OT_CURLY_BRACES_CLOSE));
					else if (eatOperator(OT_COLON))
					{
						while (eatLineBreak());
						AST::Node* n = parse();
						if (n && n->isStatement())
							decl->setGet(dynamic_cast<AST::Statement*>(n));
						else throw "inner content of set statement must be a statement!";
					}
					else throw "could not parse set operator";
				}
			}

			while (eatLineBreak());
			if (!eatOperator(OT_CURLY_BRACES_CLOSE))
				throw "'}' expected";

			return decl;
		}

		// Function declaration
		if (eatOperator(OT_PARENTHESIS_OPEN))
		{
			auto decl = new AST::FunctionDeclaration(varDecl);
			if (!eatOperator(OT_PARENTHESIS_CLOSE))
			{
				decl->addParameter(parse());
				/*do {
					AST::Node *declaration = parse(OperatorsMap::getOperatorByDefinition(OT_COMMA).second._binaryPrecedence);
					if (declaration != nullptr && declaration->isStatement()
						&& dynamic_cast<AST::Statement*>(declaration)->getStatementType() == ST_VARIABLE_DECLARATION)
						decl->addParameter(dynamic_cast<AST::VariableDeclaration*>(declaration));
					else throw "only variable declarations are legal in function declaration parenthesis section.";
				} while (eatOperator(OT_COMMA));*/
				if (!eatOperator(OT_PARENTHESIS_CLOSE))
					throw "missing ')'";
			}
			if (eatOperator(OT_CURLY_BRACES_OPEN))
				decl->setContent(parseBlock(OT_CURLY_BRACES_CLOSE));
			else if (eatOperator(OT_COLON))
			{
				while (eatLineBreak());
				AST::Node* n = parse();
				if (n && n->isStatement())
				{
					auto content = new AST::StatementBlock();
					content->addStatement(dynamic_cast<AST::Statement*>(n));
					decl->setContent(content);
				}
				else throw "inner content of function must be a statement!";
			}
			else decl->setContent(NULL);
			return decl;
		}

		return varDecl;
	}

	if (isOperator(token, OT_COMMA))
	{
		if (left->isExpression())
		{
			auto list = new AST::ExpressionList();
			list->addExpression(dynamic_cast<AST::Expression*>(left));
			auto node = parse(precedence(token, BINARY));
			if (node->isExpression())
				list->addExpression(dynamic_cast<AST::Expression*>(node));
			else throw "ExpressionList must be all expressions!";
			return list;
		}
		else
		{
			auto list = new AST::StatementList();
			list->addStatement(dynamic_cast<AST::Statement*>(left));
			auto node = parse(precedence(token, BINARY));
			if (node->isExpression())
				list->addStatement(dynamic_cast<AST::Statement*>(node));
			else throw "StatementList must be all statements!";
			return list;
		}
	}

	if (fromCategory(token, BINARY))
	{
		auto ot = ((OperatorToken*)token);

		if (ot->_operator._type == OT_PARENTHESIS_OPEN)
		{	
			if (left->isExpression())
			{
				auto funcCall = new AST::FunctionCall();
				funcCall->setFunctionId(dynamic_cast<AST::Expression*>(left));
				while (!eatOperator(OT_PARENTHESIS_CLOSE))
				{
					auto exp = parse(10);
					if (exp && !exp->isExpression())
						throw "Could not convert from Node* to Expression*";
					funcCall->addParameter(dynamic_cast<AST::Expression*>(exp));
					if (isOperator(peekToken(), OT_COMMA))
						nextToken();
				}
				return funcCall;
			}
			else throw "Expression preceding parenthesis of apparent call must be an expression";
		}
		if (OperatorsMap::isAssignment(ot->_operator._type))
		{
			auto op = new AST::Assignment();
			op->setOperator(ot->_operator);
			int prec = precedence(ot, BINARY);
			if (ot->_operator._associativity == RIGHT_TO_LEFT) prec--;
			try { op->setLeft(dynamic_cast<AST::Expression*>(left)); }
			catch (exception) { throw "Could not convert from Node* to Expression*"; }	// Should be a DinoException in the future
			try { op->setRight(dynamic_cast<AST::Expression*>(parse(prec))); }
			catch (exception) { throw "Could not convert from Node* to Expression*"; }	// Should be a DinoException in the future
			return op;
		}

		auto op = new AST::BinaryOperation();
		op->setOperator(ot->_operator);
		int prec = precedence(ot, BINARY);
		if (ot->_operator._associativity == RIGHT_TO_LEFT) prec--;
		try { op->setLeft(dynamic_cast<AST::Expression*>(left)); }
		catch (exception) { throw "Could not convert from Node* to Expression*"; }	// Should be a DinoException in the future


		if (ot->_operator._type == OT_SQUARE_BRACKETS_OPEN)
		{
			prec = 0;
			if (eatOperator(OT_SQUARE_BRACKETS_CLOSE))
			{
				op->setRight(NULL);
				return op;
			}
		}

		try { op->setRight(dynamic_cast<AST::Expression*>(parse(prec))); }
		catch (exception) { throw "Could not convert from Node* to Expression*"; }	// Should be a DinoException in the future

		if (ot->_operator._type == OT_SQUARE_BRACKETS_OPEN && eatOperator(OT_SQUARE_BRACKETS_CLOSE))
			nextToken(OT_SQUARE_BRACKETS_CLOSE); // TODO - should be "parseBlock" (should it???)

		return op;
	}

	if (fromCategory(token, POSTFIX)) 
	{
		auto op = new AST::UnaryOperation();
		op->setIsPostfix(true);
		op->setOperator(((OperatorToken*)token)->_operator);
		int prec = precedence(token, POSTFIX);
		if (op->getOperator()._associativity == RIGHT_TO_LEFT) prec--;
		try { op->setExpression(dynamic_cast<AST::Expression*>(left)); }
		catch (exception) { throw "Could not convert from Node* to Expression*"; }	// Should be a DinoException in the future
		return op;
	}

	throw "led could not find an option";
}
