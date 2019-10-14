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
	if (peekToken())
		block->setLine(peekToken()->_line);
	else block->setLine(-1);
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
			else throw DinoException("unused expression", EXT_GENERAL, node->getLine());
		}

		if (eatOperator(expected))
			return block;
		nextToken();
	}
	return block;
}

#define fromCategory(tok, cat) (tok->_type == TT_OPERATOR && precedence(tok, cat) != NONE)

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

AST::Node * Parser::parse(int lastPrecedence)
{
	Token* tok = nextToken();

	if (isOperator(tok, OT_EOF) || isOperator(tok, OT_PARENTHESIS_CLOSE) || isOperator(tok, OT_CURLY_BRACES_CLOSE) || isOperator(tok, OT_SQUARE_BRACKETS_CLOSE))
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

AST::Node * Parser::std(Token * token)
{
	if (token->_type == TT_OPERATOR && OperatorsMap::isKeyword(((OperatorToken*)token)->_operator))
	{
		auto ot = ((OperatorToken*)token);
		if (ot->_operator._type == OT_WHILE)
		{
			AST::WhileLoop * node = new AST::WhileLoop();
			node->setLine(token->_line);
			AST::Node* inner = parse();
			if (inner->isExpression())
				node->setCondition(dynamic_cast<AST::Expression*>(inner));
			else throw DinoException("could not convert from Node* to Expression*", EXT_GENERAL, inner->getLine());

			while (eatLineBreak());

			if (eatOperator(OT_CURLY_BRACES_OPEN))
				node->setStatement(parseBlock(OT_CURLY_BRACES_CLOSE));
			else if (eatOperator(OT_COLON))
			{
				while (eatLineBreak());
				AST::Node* n = parse();
				if (n && n->isStatement())
					node->setStatement(dynamic_cast<AST::Statement*>(n));
				else throw DinoException("inner content of while statement must be a statement.", EXT_GENERAL, n->getLine());
			}

			else throw DinoException("could not parse while loop.", EXT_GENERAL, node->getLine());

			return node;
		}
		if (ot->_operator._type == OT_FOR)
		{
			AST::StatementBlock *node = new AST::StatementBlock();
			node->setLine(token->_line);

			/*if (!eatLineBreak())
			{
				do
				{
					AST::Node *declaration = parse(OperatorsMap::getOperatorByDefinition(OT_COMMA).second._binaryPrecedence);
					if (declaration->isStatement())
						node->addStatement(dynamic_cast<AST::Statement*>(declaration));
					else throw "for's decleration statement failed";
				} while (eatOperator(OT_COMMA));
			}*/

			// currently "for a = 1, b = 2 | true | a++" wouldn't work, since the comma has a higher precedence than an assignment
			AST::Node* declaration = parse();
			if (declaration->isStatement())
				node->addStatement(dynamic_cast<AST::Statement*>(declaration));
			else throw DinoException("for's declaration statement failed.", EXT_GENERAL, declaration->getLine());

			AST::WhileLoop * whileNode = new AST::WhileLoop();
			whileNode->setLine(token->_line);

			if (!eatLineBreak())
				throw DinoException("missing conditional of for loop.", EXT_GENERAL, whileNode->getLine());

			AST::Node* inner = parse();
			if (inner->isExpression())
				whileNode->setCondition(dynamic_cast<AST::Expression*>(inner));
			else throw DinoException("could not convert from Node* to Expression*.", EXT_GENERAL, inner->getLine());

			if (!eatLineBreak())
				throw DinoException("missing 3rd part of for loop.", EXT_GENERAL, whileNode->getLine());

			//vector<AST::Node *> increments;
			//do
			//{
			//	//AST::Node *increment = parse(OperatorsMap::getOperatorByDefinition(OT_COMMA).second._binaryPrecedence);
			//	AST::Node *increment = parse();
			//	if (increment->getNodeId())
			//		increments.push_back(increment);
			//	else throw "for's decleration statement failed";
			//} while (eatOperator(OT_COMMA));

			vector<AST::Statement *> increments;
			do { 
				AST::Node* n = parse();
				if (n == nullptr || n->isStatement())
					increments.push_back(dynamic_cast<AST::Statement*>(n)); 
				else throw DinoException("third part of for loop must be a statement", EXT_GENERAL, whileNode->getLine());
			} 
			while (eatOperator(OT_COMMA));

			while (eatLineBreak());

			AST::StatementBlock *innerBlock = nullptr;
			if (eatOperator(OT_CURLY_BRACES_OPEN))
				innerBlock = parseBlock(OT_CURLY_BRACES_CLOSE);
			else if (eatOperator(OT_COLON))
			{
				innerBlock = new AST::StatementBlock();
				innerBlock->setLine(token->_line);
				while (eatLineBreak());
				AST::Node* n = parse();
				if (n && n->isStatement())
					innerBlock->addStatement(dynamic_cast<AST::Statement*>(n));
				else throw DinoException("inner content of a for loop's statement must be a statement.", EXT_GENERAL, n->getLine());
			}
			else throw DinoException("expected a statement", EXT_GENERAL, whileNode->getLine());

			for (auto increment : increments)
				innerBlock->addStatement(dynamic_cast<AST::Statement*>(increment));

			whileNode->setStatement(innerBlock);

			node->addStatement(whileNode);
			return node;
		}
		if (ot->_operator._type == OT_DO)
		{
			AST::DoWhileLoop * node = new AST::DoWhileLoop();
			node->setLine(token->_line);

			while (eatLineBreak());

			if (eatOperator(OT_CURLY_BRACES_OPEN))
				node->setStatement(parseBlock(OT_CURLY_BRACES_CLOSE));
			else if (eatOperator(OT_COLON))
			{
				while (eatLineBreak());
				AST::Node* n = parse();
				if (n && n->isStatement())
					node->setStatement(dynamic_cast<AST::Statement*>(n));
				else throw DinoException("inner content of do-while statement must be a statement!", EXT_GENERAL, n->getLine());
			}
			else throw DinoException("could not do parse do-while loop", EXT_GENERAL, node->getLine());

			while (eatLineBreak());
			if (!eatOperator(OT_WHILE))
				throw DinoException("could not do parse do-while loop", EXT_GENERAL, peekToken()->_line);

			AST::Node* inner = parse();
			if (inner->isExpression())
				node->setCondition(dynamic_cast<AST::Expression*>(inner));
			else throw DinoException("could not convert from Node* to Expression*.", EXT_GENERAL, inner->getLine());

			return node;
		}
		if (ot->_operator._type == OT_IF)
		{
			AST::IfThenElse * node = new AST::IfThenElse();
			node->setLine(token->_line);
			AST::Node* inner = parse();

			if (inner && inner->isExpression())
				node->setCondition(dynamic_cast<AST::Expression*>(inner));
			else throw DinoException("could not convert from Node* to Expression*.", EXT_GENERAL, inner->getLine());

			while (eatLineBreak());

			if (eatOperator(OT_CURLY_BRACES_OPEN))
				node->setThenBranch(parseBlock(OT_CURLY_BRACES_CLOSE));
			else if (eatOperator(OT_COLON))
			{
				while (eatLineBreak());
				AST::Node* n = parse();
				if (n && n->isStatement())
					node->setThenBranch(dynamic_cast<AST::Statement*>(n));
				else throw DinoException("expected a statement.", EXT_GENERAL, n->getLine());
			}
			else throw DinoException("could not then part of if statement.", EXT_GENERAL, node->getLine());

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
				else throw DinoException("expected a statement.", EXT_GENERAL, p->getLine());
			}
			else if (b) _index--;

			return node;
		}
		if (ot->_operator._type == OT_UNLESS)
		{
			AST::IfThenElse * node = new AST::IfThenElse();
			node->setLine(token->_line);
			AST::Node* inner = parse();

			if (inner && inner->isExpression())
				node->setCondition(dynamic_cast<AST::Expression*>(inner));
			else throw DinoException("expected an expression.", EXT_GENERAL, inner->getLine());

			while (eatLineBreak());

			if (eatOperator(OT_CURLY_BRACES_OPEN))
				node->setElseBranch(parseBlock(OT_CURLY_BRACES_CLOSE));
			else if (eatOperator(OT_COLON))
			{
				while (eatLineBreak());
				AST::Node* n = parse();
				if (n && n->isStatement())
					node->setElseBranch(dynamic_cast<AST::Statement*>(n));
				else throw DinoException("expected a statement.", EXT_GENERAL, n->getLine());
			}
			else throw DinoException("missing inner statement.", EXT_GENERAL, node->getLine());

			return node;
		}
		if (ot->_operator._type == OT_TYPE)
		{
			auto decl = new AST::TypeDeclaration();
			decl->setLine(token->_line);
			if (peekToken()->_type == TT_IDENTIFIER)
				decl->setName(nextToken()->_data);
			else throw DinoException("could not parse type declaration.", EXT_GENERAL, decl->getLine());

			if (eatOperator(OT_IS))
				do
				{
					if (peekToken()->_type != TT_IDENTIFIER)
						throw DinoException("incorrect use of 'is' operator inside type declaration.", EXT_GENERAL, peekToken()->_line);
					decl->addInterface(nextToken()->_data);
				} while (eatOperator(OT_COMMA));

			while (eatLineBreak());
			if (!eatOperator(OT_CURLY_BRACES_OPEN))
				throw DinoException("'{' expected.", EXT_GENERAL, decl->getLine());

			while (eatLineBreak());

			while (!eatOperator(OT_CURLY_BRACES_CLOSE))
			{
				auto temp = parse();
				if (!temp->isStatement())
					throw DinoException("expected a statement.", EXT_GENERAL, temp->getLine());
				switch (dynamic_cast<AST::Statement*>(temp)->getStatementType())
				{
				case(ST_VARIABLE_DECLARATION): decl->addVariableDeclaration(dynamic_cast<AST::VariableDeclaration*>(temp)); break;
				case(ST_FUNCTION_DECLARATION): decl->addFunctionDeclaration(dynamic_cast<AST::FunctionDeclaration*>(temp)); break;
				case(ST_PROPERTY_DECLARATION): decl->addPropertyDeclaration(dynamic_cast<AST::PropertyDeclaration*>(temp)); break;
				}
				while(eatLineBreak());
			}
			return decl;
		}
		if (ot->_operator._type == OT_INTERFACE)
		{
			auto decl = new AST::InterfaceDeclaration();
			decl->setLine(token->_line);
			if (peekToken()->_type == TT_IDENTIFIER)
				decl->setName({ nextToken()->_data });
			else throw DinoException("identificator expected.", EXT_GENERAL, decl->getLine());

			eatLineBreak();
			if (eatOperator(OT_IS)) 
				do 
				{
					if (peekToken()->_type != TT_IDENTIFIER)
						throw DinoException("incorrect use of 'is' operator inside interface declaration.", EXT_GENERAL, peekToken()->_line);
					decl->addImplements({ nextToken()->_data });
				} while (eatOperator(OT_COMMA));

			eatLineBreak();
			if (!eatOperator(OT_CURLY_BRACES_OPEN))
				throw DinoException("'{' expected.", EXT_GENERAL, peekToken()->_line);
			eatLineBreak();
			while (!eatOperator(OT_CURLY_BRACES_CLOSE))
			{
				auto temp = parse();
				if (!temp->isStatement())
					throw DinoException("excpected a statement.", EXT_GENERAL, temp->getLine());

				if (dynamic_cast<AST::Statement*>(temp)->getStatementType() == ST_VARIABLE_DECLARATION)
					decl->addProperty(dynamic_cast<AST::VariableDeclaration*>(temp));
				else if (dynamic_cast<AST::Statement*>(temp)->getStatementType() == ST_FUNCTION_DECLARATION)
					decl->addFunction(dynamic_cast<AST::FunctionDeclaration*>(temp));
				else throw DinoException("body of interface declaration may contain only declarations.", EXT_GENERAL, temp->getLine());
				eatLineBreak();
			}
			return decl;
		}
		if (ot->_operator._type == OT_NAMESPACE)
		{
			auto decl = new AST::NamespaceDeclaration();
			decl->setLine(token->_line);
			if (peekToken()->_type == TT_IDENTIFIER)
				decl->setName({ nextToken()->_data });
			else throw DinoException("expected an identifier.", EXT_GENERAL, peekToken()->_line);
			while (eatLineBreak());
			if (eatOperator(OT_CURLY_BRACES_OPEN))
				decl->setStatement(parseBlock(OT_CURLY_BRACES_CLOSE));
			else throw DinoException("'{' expected.", EXT_GENERAL, peekToken()->_line);
			return decl;
		}
		if (ot->_operator._type == OT_DELETE || ot->_operator._type == OT_RETURN)
		{
			auto op = new AST::UnaryOperationStatement();
			op->setLine(token->_line);
			op->setOperator(ot->_operator);
			//int prec = precedence(ot);
			//if (ot->_operator._associativity == RIGHT_TO_LEFT) prec--;
			//try { op->setExpression(dynamic_cast<AST::Expression*>(parse(prec))); }
			try { op->setExpression(dynamic_cast<AST::Expression*>(parse())); }
			catch (exception) { throw DinoException("expected an expression.", EXT_GENERAL, peekToken()->_line); }
			return op;
		}
	}
	return NULL;
}

AST::Node * Parser::nud(Token * token)
{
	if (token->_type == TT_IDENTIFIER)
	{
		auto node = new AST::Variable(token->_data);
		node->setLine(token->_line);
		return node;
	}
	if (token->_type == TT_LITERAL)
	{
		AST::Node* node;
		switch (((LiteralToken<int>*)token)->_literalType)
		{
		case (LT_BOOLEAN): node = new AST::Boolean(((LiteralToken<bool>*)token)->_value); break;
			case (LT_INTEGER): node = new AST::Integer(((LiteralToken<int>*)token)->_value); break;
			case (LT_STRING):  node = new AST::String(((LiteralToken<string>*)token)->_value); break;
			case (LT_CHARACTER): node = new AST::Character(((LiteralToken<char>*)token)->_value); break;
			case (LT_FRACTION): node = new AST::Fraction(((LiteralToken<float>*)token)->_value); break;
			case (LT_NULL): node = new AST::Null(); break;
			default: return NULL;	// TODO - proper error system
		}
		node->setLine(token->_line);
		return node;
	}
	if (isOperator(token, OT_PARENTHESIS_OPEN))
	{
		AST::Node* inner = parse();
		if (!eatOperator(OT_PARENTHESIS_CLOSE))
			throw DinoException("')' expected.", EXT_GENERAL, peekToken()->_line);

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
			func->setLine(token->_line);
			if (inner != nullptr)
				func->addParameter(inner);
			AST::Node* returnType = parse();
			if (!returnType->isExpression())
				throw DinoException("expected an expression.", EXT_GENERAL, returnType->getLine());
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
					content->setLine(token->_line);
					content->addStatement(dynamic_cast<AST::Statement*>(n));
					func->setContent(content);
				}
				else throw DinoException("expected a statement.", EXT_GENERAL, n->getLine());
			}
			else throw DinoException("missing function literal body.", EXT_GENERAL, peekToken()->_line);

			while (eatLineBreak());
			return func;
		}

		return inner;
	}
	if (fromCategory(token, PREFIX))
	{
		auto ot = ((OperatorToken*)token);
		if (isOperator(token, OT_INCREMENT) || isOperator(token, OT_DECREMENT))
		{
			auto op = new AST::UnaryAssignment();
			op->setLine(token->_line);
			op->setOperator(ot->_operator);
			int prec = precedence(ot, PREFIX);
			if (ot->_operator._associativity == RIGHT_TO_LEFT) prec--;
			try { op->setExpression(dynamic_cast<AST::Expression*>(parse(prec))); }
			catch (exception) { throw DinoException("expected an expression", EXT_GENERAL, op->getLine()); }
			return op;
		}
		auto op = new AST::UnaryOperation();
		op->setLine(token->_line);
		op->setOperator(ot->_operator);
		int prec = precedence(ot, PREFIX);
		if (ot->_operator._associativity == RIGHT_TO_LEFT) prec--;

		if (ot->_operator._type == OT_SQUARE_BRACKETS_OPEN)
		{
			prec = 0;
			if (eatOperator(OT_SQUARE_BRACKETS_CLOSE))
			{
				op->setExpression(NULL);
				return op;
			}
			try { op->setExpression(dynamic_cast<AST::Expression*>(parse(prec))); }
			catch (exception) { throw DinoException("expected an expression", EXT_GENERAL, op->getLine()); }
			if (eatOperator(OT_SQUARE_BRACKETS_CLOSE))
				return op;
			else throw DinoException("']' expected", EXT_GENERAL, peekToken()->_line);
		}
		if (ot->_operator._type == OT_CURLY_BRACES_OPEN)
		{
			prec = 0;
			if (eatOperator(OT_CURLY_BRACES_CLOSE))
			{
				op->setExpression(NULL);
				return op;
			}
			try { op->setExpression(dynamic_cast<AST::Expression*>(parse(prec))); }
			catch (exception) { throw DinoException("expected an expression", EXT_GENERAL, op->getLine()); }
			if (eatOperator(OT_CURLY_BRACES_CLOSE))
				return op;
			else throw DinoException("'}' expected", EXT_GENERAL, peekToken()->_line);
		}

		try { op->setExpression(dynamic_cast<AST::Expression*>(parse(prec))); }
		catch (exception) { throw DinoException("expected an expression", EXT_GENERAL, op->getLine()); }
		return op;
	}
	return NULL;
}

AST::Node * Parser::led(AST::Node * left, Token * token)
{
	if (left->isExpression() && token->_type == TT_IDENTIFIER)	// variable declaration
	{
		auto varDecl = new AST::VariableDeclaration();
		varDecl->setLine(token->_line);
		varDecl->setType(dynamic_cast<AST::Expression*>(left));
		varDecl->setVarId(token->_data);

		// Property declaration
		if (eatOperator(OT_CURLY_BRACES_OPEN))
		{
			

			auto decl = new AST::PropertyDeclaration(varDecl);
			decl->setLine(token->_line);

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
					else throw DinoException("expected a statement", EXT_GENERAL, n->getLine());
				}
				else throw DinoException("expected a block statement", EXT_GENERAL, peekToken()->_line);

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
						else throw DinoException("expected a statement", EXT_GENERAL, n->getLine());
					}
					else throw DinoException("expected a block statement", EXT_GENERAL, peekToken()->_line);
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
					else throw DinoException("expected a statement", EXT_GENERAL, n->getLine());
				}
				else throw DinoException("expected a block statement", EXT_GENERAL, peekToken()->_line);

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
						else throw DinoException("expected a statement", EXT_GENERAL, n->getLine());
					}
					else throw DinoException("expected a block statement", EXT_GENERAL, peekToken()->_line);
				}
			}

			while (eatLineBreak());
			if (!eatOperator(OT_CURLY_BRACES_CLOSE))
				throw DinoException("'}' exected.", EXT_GENERAL, peekToken()->_line);

			return decl;
		}

		// Function declaration
		if (eatOperator(OT_PARENTHESIS_OPEN))
		{
			auto decl = new AST::FunctionDeclaration(varDecl);
			decl->setLine(token->_line);
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
					throw DinoException("')' exected.", EXT_GENERAL, peekToken()->_line);
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
					content->setLine(token->_line);
					content->addStatement(dynamic_cast<AST::Statement*>(n));
					decl->setContent(content);
				}
				else throw DinoException("expected a statement", EXT_GENERAL, n->getLine());
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
			list->setLine(token->_line);
			list->addExpression(dynamic_cast<AST::Expression*>(left));
			auto node = parse(precedence(token, BINARY));
			/*if (node == nullptr)
				return list;*/
			if (node == NULL)
				list->addExpression(NULL);
			if (node->isExpression())
				list->addExpression(dynamic_cast<AST::Expression*>(node));
			else throw DinoException("ExpressionList may only contain expressions.", EXT_GENERAL, node->getLine());
			return list;
		}
		else
		{
			auto list = new AST::StatementList();
			list->setLine(token->_line);
			list->addStatement(dynamic_cast<AST::Statement*>(left));
			auto node = parse(precedence(token, BINARY));
			if (node->isExpression())
				list->addStatement(dynamic_cast<AST::Statement*>(node));
			else throw DinoException("StatementList may only contain statements.", EXT_GENERAL, node->getLine());
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
				funcCall->setLine(token->_line);
				funcCall->setFunctionId(dynamic_cast<AST::Expression*>(left));
				while (!eatOperator(OT_PARENTHESIS_CLOSE))
				{
					auto exp = parse(10);
					if (exp && !exp->isExpression())
						throw DinoException("expected a statement", EXT_GENERAL, exp->getLine());
					funcCall->addParameter(dynamic_cast<AST::Expression*>(exp));
					if (isOperator(peekToken(), OT_COMMA))
						nextToken();
				}
				return funcCall;
			}
			else throw DinoException("Expression preceding parenthesis of apparent call must be an expression", EXT_GENERAL, left->getLine());
		}
		if (OperatorsMap::isAssignment(ot->_operator._type))
		{
			auto op = new AST::Assignment();
			op->setLine(token->_line);
			op->setOperator(ot->_operator);
			int prec = precedence(ot, BINARY);
			if (ot->_operator._associativity == RIGHT_TO_LEFT) prec--;
			try { op->setLeft(dynamic_cast<AST::Expression*>(left)); }
			catch (exception) { throw DinoException("expected an expression", EXT_GENERAL, op->getLine()); }
			try { op->setRight(dynamic_cast<AST::Expression*>(parse(prec))); }
			catch (exception) { throw DinoException("expected an expression", EXT_GENERAL, op->getLine()); }
			return op;
		}

		auto op = new AST::BinaryOperation();
		op->setLine(token->_line);
		op->setOperator(ot->_operator);
		int prec = precedence(ot, BINARY);
		if (ot->_operator._associativity == RIGHT_TO_LEFT) prec--;
		try { op->setLeft(dynamic_cast<AST::Expression*>(left)); }
		catch (exception) { throw DinoException("expected an expression", EXT_GENERAL, op->getLine()); }

		if (ot->_operator._type == OT_SQUARE_BRACKETS_OPEN)
		{
			prec = 0;
			if (eatOperator(OT_SQUARE_BRACKETS_CLOSE))
			{
				op->setRight(NULL);
				return op;
			}
			try { op->setRight(dynamic_cast<AST::Expression*>(parse(prec))); }
			catch (exception) { throw DinoException("expected an expression", EXT_GENERAL, op->getLine()); }
			if (eatOperator(OT_SQUARE_BRACKETS_CLOSE))
				return op;
			else throw DinoException("']' expected.", EXT_GENERAL, op->getLine());
		}



		/*if (ot->_operator._type == OT_SQUARE_BRACKETS_OPEN)
		{
			prec = 0;
			if (eatOperator(OT_SQUARE_BRACKETS_CLOSE))
			{
				op->setRight(NULL);
				return op;
			}
		}*/

		try { op->setRight(dynamic_cast<AST::Expression*>(parse(prec))); }
		catch (exception) { throw DinoException("expected an expression", EXT_GENERAL, op->getLine()); }

		//if (ot->_operator._type == OT_SQUARE_BRACKETS_OPEN && eatOperator(OT_SQUARE_BRACKETS_CLOSE))
		//	nextToken(OT_SQUARE_BRACKETS_CLOSE); // TODO - should be "parseBlock" (should it???)

		return op;
	}
	if (fromCategory(token, POSTFIX)) 
	{
		if (isOperator(token, OT_INCREMENT) || isOperator(token, OT_DECREMENT)) 
		{
			auto op = new AST::UnaryAssignment();
			op->setLine(token->_line);
			op->setIsPostfix(true);
			op->setOperator(((OperatorToken*)token)->_operator);
			int prec = precedence(token, POSTFIX);
			if (op->getOperator()._associativity == RIGHT_TO_LEFT) prec--;
			try { op->setExpression(dynamic_cast<AST::Expression*>(left)); }
			catch (exception) { throw DinoException("expected an expression", EXT_GENERAL, op->getLine()); }
			return op;
		}
		auto op = new AST::UnaryOperation();
		op->setLine(token->_line);
		op->setIsPostfix(true);
		op->setOperator(((OperatorToken*)token)->_operator);
		int prec = precedence(token, POSTFIX);
		if (op->getOperator()._associativity == RIGHT_TO_LEFT) prec--;
		try { op->setExpression(dynamic_cast<AST::Expression*>(left)); }
		catch (exception) { throw DinoException("expected an expression", EXT_GENERAL, op->getLine()); }
		return op;
	}
	throw DinoException("led could not find an option.", EXT_GENERAL, token->_line);
}
