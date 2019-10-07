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
				decl->setName({ nextToken()->_data });
			else throw "could not parse type declaration";

			if (eatOperator(OT_IS))
				do
				{
					if (peekToken()->_type != TT_IDENTIFIER)
						throw "could not parse interface declaration, incorrect use of 'is' operator";
					decl->addInterface({ nextToken()->_data });
				} while (eatOperator(OT_COMMA));

			eatLineBreak();
			if (!eatOperator(OT_CURLY_BRACES_OPEN))
				throw "could not parse type declaration";
			eatLineBreak();
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
				case(ST_ASSIGNMENT):
					decl->addFunctionDeclaration(dynamic_cast<AST::Assignment*>(temp));
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
			else throw "could not parse type declaration";
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

				if (dynamic_cast<AST::Statement*>(temp)->getStatementType() != ST_VARIABLE_DECLARATION)
					throw "body of interface declaration may contain only declarations";
				else decl->addDeclaration(dynamic_cast<AST::VariableDeclaration*>(temp));
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
			int prec = ot->_operator._precedence;
			if (ot->_operator._associativity == RIGHT_TO_LEFT) prec--;
			try { op->setExpression(dynamic_cast<AST::Expression*>(parse(prec))); }
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

			if (eatOperator(OT_CURLY_BRACES_OPEN))
			{
				// Propery declaration: 
				auto decl = new AST::PropertyDeclaration(node);

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
			case (LT_STRING):  return new AST::String(((LiteralToken<string>*)token)->_value);
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
			if (inner == nullptr)
				_index--;
			if (!eatOperator(OT_PARENTHESIS_CLOSE))
				throw "')' missing";
			if (peekToken()->_type == TT_IDENTIFIER)
			{
				// Function literal
				AST::Identificator returnType = { nextToken()->_data };

				auto func = new AST::Function();
				vector<AST::VariableDeclaration*> vec;

				AST::Node* temp = inner;
				while (temp && temp->isExpression() && dynamic_cast<AST::Expression*>(temp)->getExpressionType() == ET_BINARY_OPERATION)
				{
					auto bo = dynamic_cast<AST::BinaryOperation*>(temp);
					if (bo->getOperator()._type == OT_COMMA)
						if (bo->getRight()->isStatement() && dynamic_cast<AST::Statement*>(bo->getRight())->getStatementType() == ST_VARIABLE_DECLARATION)
							vec.push_back(dynamic_cast<AST::VariableDeclaration*>(bo->getRight()));
						else throw "TODO - error msg";

					/*if (bo->getLeft()->isStatement())
						if (dynamic_cast<AST::Statement*>(bo->getLeft())->getStatementType() == ST_VARIABLE_DECLARATION)
							vec.push_back(dynamic_cast<AST::VariableDeclaration*>(bo->getLeft()));
						else throw "TODO - error msg";*/

					temp = bo->getLeft();
					delete bo;
				}
				if (temp && temp->isStatement())
					if (dynamic_cast<AST::Statement*>(temp)->getStatementType() == ST_VARIABLE_DECLARATION)
						vec.push_back(dynamic_cast<AST::VariableDeclaration*>(temp));
					else throw "TODO - error msg";

				std::reverse(vec.begin(), vec.end());

				for (auto i : vec)
					func->addParameter(i);
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
				
				func->setReturnType(returnType);
				while (eatLineBreak());
				return func;
			}
			
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
	if (left->isStatement() && dynamic_cast<AST::Statement*>(left)->getStatementType() == ST_VARIABLE_DECLARATION
		&& token->_type == TT_OPERATOR && isOperator(token, OT_CURLY_BRACES_OPEN))
	{
		// Propery declaration: 
		auto decl = new AST::PropertyDeclaration(dynamic_cast<AST::VariableDeclaration*>(left));

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
					decl->setGet(parseBlock(OT_CURLY_BRACES_CLOSE));
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

	if (token->_type == TT_OPERATOR && OperatorsMap::isBinary(((OperatorToken*)token)->_operator._type))
	{
		auto ot = ((OperatorToken*)token);

		if (ot->_operator._type == OT_PARENTHESIS_OPEN)
		{	
			if (left->isExpression() && dynamic_cast<AST::Expression*>(left)->getExpressionType() == ET_VARIABLE_DECLARATION)
			{
				// Function declaration
				while (eatLineBreak());
				auto decl = dynamic_cast<AST::VariableDeclaration*>(left);
				auto assign = new AST::Assignment();
				auto func = new AST::Function;
				
				do
				{
					AST::Node *declaration = parse(OperatorsMap::getOperatorByDefinition(OT_COMMA).second._precedence);
					if (declaration != nullptr && declaration->isStatement() && 
						dynamic_cast<AST::Statement*>(declaration)->getStatementType() == ST_VARIABLE_DECLARATION)
						func->addParameter(dynamic_cast<AST::VariableDeclaration*>(declaration));
					else
					{
						_index--;
						break;
					}
					//else throw "for's decleration statement failed";
				} while (eatOperator(OT_COMMA));
				
				if (!eatOperator(OT_PARENTHESIS_CLOSE))
					throw "missing ')'";

				while (eatLineBreak());

				if (eatOperator(OT_CURLY_BRACES_OPEN)) 
					func->setContent(parseBlock(OT_CURLY_BRACES_CLOSE));
				else if (eatOperator(OT_COLON))
				{
					while (eatLineBreak());
					AST::Node* n = parse();
					if (n && n->isStatement())
					{
						auto content = new AST::StatementBlock();
						content->addStatement(dynamic_cast<AST::Statement*>(n));
						func->setContent(content);
					}
					else throw "inner content of function must be a statement!";
				}
				else throw "missing function body.";

				

				decl->addModifier({ "func" });
				assign->setLeft(decl);
				func->setReturnType(decl->getVarType());
				assign->setRight(func);
				assign->setOperator(OperatorsMap::getOperatorByDefinition(OT_ASSIGN_EQUAL).second);
				
				return assign;
			}
			/*if (left->isExpression() && dynamic_cast<AST::Expression*>(left)->getExpressionType() == ET_VARIABLE)
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
			}*/
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

	if (token->_type == TT_OPERATOR && (((OperatorToken*)token)->_operator._type == OT_INCREMENT || ((OperatorToken*)token)->_operator._type == OT_DECREMENT))
		throw "postfix increment/decrement is not supported yet.";

	throw "led could not find an option";
	return NULL;	// Error
}
