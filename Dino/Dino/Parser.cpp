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
		eatLineBreak();
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
		eatLineBreak();
	}
	return block;
}

void Parser::expectLineBreak()
{
	if (!eatLineBreak())
		throw DinoException("expected a line break", EXT_GENERAL, peekToken()->_line);
}

void Parser::expectOperator(OperatorType ot)
{
	if (!eatOperator(ot)) 
	{
		string errorMsg = "expected '" + OperatorsMap::getOperatorByDefinition(ot).second._str + "'";
		throw DinoException(errorMsg.c_str(), EXT_GENERAL, peekToken()->_line);
	}
}

string Parser::expectIdentifier()
{
	if (peekToken()->_type == TT_IDENTIFIER)
		return nextToken()->_data;
	else throw DinoException("expected an identifier.", EXT_GENERAL, peekToken()->_line);
}

vector<string> Parser::expectIdentifierList()
{
	vector<string> v;
	v.push_back(expectIdentifier());
	while (eatOperator(OT_COMMA));
		v.push_back(expectIdentifier());
	return v;
}

AST::Statement * Parser::parseStatement()
{
	/*if (isOperator(peekToken(), OT_CURLY_BRACES_OPEN))
		return NULL;*/
	AST::Node* node = parse();
	if (node == nullptr || node->isStatement())
		return dynamic_cast<AST::Statement*>(node);
	else throw DinoException("expected a statement", EXT_GENERAL, node->getLine());
}

AST::Expression * Parser::parseExpression()
{
	AST::Node* node = parse();
	if (node != nullptr && node->isExpression())
		return dynamic_cast<AST::Expression*>(node);
	else throw DinoException("expected an expression", EXT_GENERAL, node->getLine());
}

AST::Statement * Parser::parseInnerBlock()
{
	if (eatOperator(OT_CURLY_BRACES_OPEN))
		return parseBlock(OT_CURLY_BRACES_CLOSE);
	else if (eatOperator(OT_COLON))
	{
		//while (eatLineBreak());
		AST::Node* n = parse();
		if (n && n->isStatement())
			return dynamic_cast<AST::Statement*>(n);
		else throw DinoException("expected a statement.", EXT_GENERAL, n->getLine());
	}
	else throw DinoException("expected a block statement.", EXT_GENERAL, peekToken()->_line);
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
	if (peekToken()->_type == TT_LINE_BREAK || isOperator(peekToken(), OT_SQUARE_BRACKETS_CLOSE)
		|| isOperator(peekToken(), OT_PARENTHESIS_CLOSE) || isOperator(peekToken(), OT_CURLY_BRACES_CLOSE) 
		|| isOperator(peekToken(), OT_EOF) || isOperator(peekToken(), OT_COLON))
		return NULL;

	Token* tok = nextToken();
		
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
			auto node = new AST::WhileLoop();
			node->setCondition(parseExpression());
			node->setStatement(parseInnerBlock());
			node->setLine(token->_line);
			return node;
			/*AST::WhileLoop * node = new AST::WhileLoop();
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

			return node;*/
		}
		if (ot->_operator._type == OT_FOR)
		{
			auto node = new AST::ForLoop();
			node->setBegin(parseStatement());
			expectLineBreak();
			node->setCondition(parseExpression());
			expectLineBreak();
			node->setIncrement(parseStatement());
			node->setStatement(parseInnerBlock());
			node->setLine(token->_line);
			return node;
		}
		if (ot->_operator._type == OT_DO)
		{
			auto node = new AST::DoWhileLoop();
			node->setStatement(parseInnerBlock());
			skipLineBreaks();
			expectOperator(OT_WHILE);
			node->setCondition(parseExpression());
			node->setLine(token->_line);
			return node;
			/*AST::DoWhileLoop * node = new AST::DoWhileLoop();
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

			return node;*/
		}
		if (ot->_operator._type == OT_IF)
		{
			auto node = new AST::IfThenElse();
			node->setCondition(parseExpression());
			node->setThenBranch(parseInnerBlock());
			skipLineBreaks();
			if (eatOperator(OT_ELSE))
				node->setElseBranch(isOperator(peekToken(), OT_IF) ? dynamic_cast<AST::IfThenElse*>(parse()) : parseInnerBlock());
			node->setLine(token->_line);
			return node;	
			/*AST::IfThenElse * node = new AST::IfThenElse();
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
			else throw DinoException("expected a block statement.", EXT_GENERAL, peekToken()->_line);

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

			return node;*/
		}
		if (ot->_operator._type == OT_UNLESS)
		{
			auto node = new AST::IfThenElse();
			node->setCondition(parseExpression());
			node->setElseBranch(parseInnerBlock());
			node->setLine(token->_line);
			return node;
			/*AST::IfThenElse * node = new AST::IfThenElse();
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

			return node;*/
		}
		if (ot->_operator._type == OT_TYPE)
		{
			auto node = new AST::TypeDeclaration();
			node->setName(expectIdentifier());
			if (eatOperator(OT_IS))
			{
				node->addInterface(expectIdentifier());
				while (eatOperator(OT_COMMA))
					node->addInterface(expectIdentifier());
			}
			expectOperator(OT_CURLY_BRACES_OPEN);
			while (!eatOperator(OT_CURLY_BRACES_CLOSE))
			{
				auto decl = parse();
				if (!decl->isStatement())
					throw DinoException("expected a statement.", EXT_GENERAL, decl->getLine());
				switch (dynamic_cast<AST::Statement*>(decl)->getStatementType())
				{
					case(ST_VARIABLE_DECLARATION): node->addVariableDeclaration(dynamic_cast<AST::VariableDeclaration*>(decl)); break;
					case(ST_FUNCTION_DECLARATION): node->addFunctionDeclaration(dynamic_cast<AST::FunctionDeclaration*>(decl)); break;
					case(ST_PROPERTY_DECLARATION): node->addPropertyDeclaration(dynamic_cast<AST::PropertyDeclaration*>(decl)); break;
					default: throw DinoException("expected a variable, property or function declaration", EXT_GENERAL, decl->getLine());
				}
				skipLineBreaks();
			}
			node->setLine(token->_line);
			return node;

			/*if (peekToken()->_type == TT_IDENTIFIER)
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
			return decl;*/
		}
		if (ot->_operator._type == OT_INTERFACE)
		{
			auto node = new AST::InterfaceDeclaration();
			node->setName(expectIdentifier());
			if (eatOperator(OT_IS))
			{
				node->addImplements(expectIdentifier());
				while (eatOperator(OT_COMMA))
					node->addImplements(expectIdentifier());
			}
			expectOperator(OT_CURLY_BRACES_OPEN);
			while (!eatOperator(OT_CURLY_BRACES_CLOSE))
			{
				auto decl = parse();
				if (!decl->isStatement())
					throw DinoException("expected a statement.", EXT_GENERAL, decl->getLine());
				switch (dynamic_cast<AST::Statement*>(decl)->getStatementType())
				{
				case(ST_VARIABLE_DECLARATION): node->addProperty(dynamic_cast<AST::VariableDeclaration*>(decl)); break;
				case(ST_FUNCTION_DECLARATION): node->addFunction(dynamic_cast<AST::FunctionDeclaration*>(decl)); break;
				default: throw DinoException("expected a property or function declaration", EXT_GENERAL, decl->getLine());
				}
				skipLineBreaks();
			}
			node->setLine(token->_line);
			return node;

			/*auto decl = new AST::InterfaceDeclaration();
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
			return decl;*/
		}
		if (ot->_operator._type == OT_NAMESPACE)
		{
			auto node = new AST::NamespaceDeclaration();
			node->setName(expectIdentifier());
			node->setStatement(parseInnerBlock());	// technically incorrect, should only accept declarations
			node->setLine(token->_line);
			return node;

			/*auto decl = new AST::NamespaceDeclaration();
			decl->setLine(token->_line);
			if (peekToken()->_type == TT_IDENTIFIER)
				decl->setName({ nextToken()->_data });
			else throw DinoException("expected an identifier.", EXT_GENERAL, peekToken()->_line);
			while (eatLineBreak());
			if (eatOperator(OT_CURLY_BRACES_OPEN))
				decl->setStatement(parseBlock(OT_CURLY_BRACES_CLOSE));
			else throw DinoException("'{' expected.", EXT_GENERAL, peekToken()->_line);
			return decl;*/
		}
		if (ot->_operator._type == OT_DELETE || ot->_operator._type == OT_RETURN)
		{
			auto op = new AST::UnaryOperationStatement();
			op->setOperator(ot->_operator);
			op->setExpression(parseExpression());
			op->setLine(token->_line);
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
	//if (isOperator())
	
	throw DinoException("nud couldn't find an option", EXT_GENERAL, token->_line);
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

					decl->setGet(parseInnerBlock());

					/*if (eatOperator(OT_CURLY_BRACES_OPEN))
						decl->setGet(parseBlock(OT_CURLY_BRACES_CLOSE));
					else if (eatOperator(OT_COLON))
					{
						while (eatLineBreak());
						AST::Node* n = parse();
						if (n && n->isStatement())
							decl->setGet(dynamic_cast<AST::Statement*>(n));
						else throw DinoException("expected a statement", EXT_GENERAL, n->getLine());
					}
					else throw DinoException("expected a block statement", EXT_GENERAL, peekToken()->_line);*/
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
			if (node == nullptr)
				list->addExpression(NULL);
			else if (node->isExpression())
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
