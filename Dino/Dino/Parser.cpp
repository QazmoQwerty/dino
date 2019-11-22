#include "Parser.h"

int _lineNum;
int _index;

Token * Parser::getToken(unsigned int index)
{
	if (index >= _tokens.size())
		return NULL;
	return _tokens[index];
}

AST::StatementBlock * Parser::parseBlock(OperatorType expected)
{
	if (peekToken() == nullptr)
		return NULL;
	auto block = new AST::StatementBlock();
	block->setLine(peekToken()->_line);
	while (peekToken() && !eatOperator(expected))
	{
		block->addStatement(convertToStatement(parse()));
		if (eatOperator(expected)) return block;
		expectLineBreak();
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
		string errorMsg = "expected '" + OperatorsMap::getOperatorByDefinition(ot).second._str.to_string() + "'";
		throw DinoException(errorMsg.c_str(), EXT_GENERAL, peekToken()->_line);
	}
}

unicode_string Parser::expectIdentifier()
{
	if (peekToken()->_type == TT_IDENTIFIER)
		return nextToken()->_data;
	else throw DinoException("expected an identifier.", EXT_GENERAL, peekToken()->_line);
}

vector<unicode_string> Parser::expectIdentifierList()
{
	vector<unicode_string> v;
	v.push_back(expectIdentifier());
	while (eatOperator(OT_COMMA));
		v.push_back(expectIdentifier());
	return v;
}

AST::Expression * Parser::convertToExpression(AST::Node * node)
{
	if (node == nullptr || node->isExpression())
		return dynamic_cast<AST::Expression*>(node);
	throw DinoException("expected an expression", EXT_GENERAL, node->getLine());
}

AST::Statement * Parser::convertToStatement(AST::Node * node)
{
	if (node == nullptr || node->isStatement())
		return dynamic_cast<AST::Statement*>(node);
	throw DinoException("expected a statement", EXT_GENERAL, node->getLine());
}

AST::Statement * Parser::parseStatement()
{
	return convertToStatement(parse());
}

AST::Expression * Parser::parseExpression(int precedence)
{
	AST::Node* node = parse(precedence);
	if (node != nullptr && node->isExpression())
		return dynamic_cast<AST::Expression*>(node);
	else throw DinoException("expected an expression", EXT_GENERAL, node ? node->getLine() : peekToken()->_line);
}

AST::Expression * Parser::parseOptionalExpression(int precedence)
{
	return convertToExpression(parse(precedence));
}

AST::Statement * Parser::parseInnerBlock()
{
	if (eatOperator(OT_CURLY_BRACES_OPEN))
		return parseBlock(OT_CURLY_BRACES_CLOSE);
	else if (eatOperator(OT_COLON))
	{
		skipLineBreaks();
		return convertToStatement(parse());
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

int Parser::leftPrecedence(OperatorToken * token, int category)
{
	int prec = precedence(token, category);
	if (token->_operator._associativity == RIGHT_TO_LEFT) prec--;
	return prec;
}

AST::Node * Parser::parse(int lastPrecedence)
{
	if (   peekToken()->_type == TT_LINE_BREAK		     || isOperator(peekToken(), OT_SQUARE_BRACKETS_CLOSE)
		|| isOperator(peekToken(), OT_PARENTHESIS_CLOSE) || isOperator(peekToken(), OT_CURLY_BRACES_CLOSE) 
		|| isOperator(peekToken(), OT_EOF)				 || isOperator(peekToken(), OT_COLON))
		return NULL;

	Token* tok = nextToken();
		
	AST::Node* left = std(tok);
	if (left) return left;

	left = nud(tok);
	if (left == NULL) return NULL;

	while (peekToken()->_type != TT_LINE_BREAK && !isOperator(peekToken(), OT_EOF) && !isOperator(peekToken(), OT_CURLY_BRACES_OPEN)
		&& (peekToken()->_type == TT_IDENTIFIER || precedence(peekToken(), BINARY | POSTFIX) > lastPrecedence))
		left = led(left, nextToken());
	return left;
}

AST::Node * Parser::std(Token * token)
{
	if (token->_type == TT_OPERATOR && OperatorsMap::isKeyword(((OperatorToken*)token)->_operator))
	{
		switch (((OperatorToken*)token)->_operator._type)	
		{
			case(OT_WHILE): {
				auto node = new AST::WhileLoop();
				node->setCondition(parseExpression());
				node->setStatement(parseInnerBlock());
				node->setLine(token->_line);
				return node;
			}
			case(OT_FOR): {
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
			case(OT_DO): {
				auto node = new AST::DoWhileLoop();
				node->setStatement(parseInnerBlock());
				skipLineBreaks();
				expectOperator(OT_WHILE);
				node->setCondition(parseExpression());
				node->setLine(token->_line);
				return node;
			}
			case(OT_IF): {
				auto node = new AST::IfThenElse();
				node->setCondition(parseExpression());
				node->setThenBranch(parseInnerBlock());
				bool b = eatLineBreak();
				if (eatOperator(OT_ELSE))
					node->setElseBranch(isOperator(peekToken(), OT_IF) ? dynamic_cast<AST::IfThenElse*>(parse()) : parseInnerBlock());
				else if (b) _index--;
				node->setLine(token->_line);
				return node;
			}
			case(OT_SWITCH): {
				auto node = new AST::SwitchCase();
				if (!eatOperator(OT_CURLY_BRACES_OPEN))
				{
					node->setCondition(parseExpression());
					expectOperator(OT_CURLY_BRACES_OPEN);
				}
				while (!eatOperator(OT_CURLY_BRACES_CLOSE))
				{
					if (eatOperator(OT_CASE))
					{
						auto expression = parseExpression();
						node->addCase(expression, parseInnerBlock());
					}
					else if (eatOperator(OT_DEFAULT))
						node->setDefault(parseInnerBlock());
					else throw DinoException("expected 'case' or 'default'", EXT_GENERAL, peekToken()->_line);

					if (!isOperator(peekToken(), OT_CURLY_BRACES_CLOSE))
						expectLineBreak();
				}
				node->setLine(token->_line);
				return node;
			}
			case(OT_UNLESS): {
				auto node = new AST::IfThenElse();
				node->setCondition(parseExpression());
				node->setElseBranch(parseInnerBlock());
				node->setLine(token->_line);
				return node;
			}
			case(OT_TYPE): {
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
					auto decl = parseStatement();
					switch (decl->getStatementType())
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
			}
			case(OT_INTERFACE):	{
				auto node = new AST::InterfaceDeclaration();
				node->setName(expectIdentifier());
				if (eatOperator(OT_IS))
				{
					node->addImplements(expectIdentifier());
					while (eatOperator(OT_COMMA))
						node->addImplements(expectIdentifier());
				}
				if (eatOperator(OT_COLON))
				{
					auto decl = parseStatement();
					switch (decl->getStatementType())
					{
					case(ST_VARIABLE_DECLARATION): node->addProperty(dynamic_cast<AST::VariableDeclaration*>(decl)); break;
					case(ST_FUNCTION_DECLARATION): node->addFunction(dynamic_cast<AST::FunctionDeclaration*>(decl)); break;
					default: throw DinoException("expected property or function declaration", EXT_GENERAL, decl->getLine());
					}
				}
				else {
					expectOperator(OT_CURLY_BRACES_OPEN);
					while (!eatOperator(OT_CURLY_BRACES_CLOSE))
					{
						auto decl = parseStatement();
						switch (decl->getStatementType())
						{
						case(ST_VARIABLE_DECLARATION): node->addProperty(dynamic_cast<AST::VariableDeclaration*>(decl)); break;
						case(ST_FUNCTION_DECLARATION): node->addFunction(dynamic_cast<AST::FunctionDeclaration*>(decl)); break;
						default: throw DinoException("expected property or function declaration", EXT_GENERAL, decl->getLine());
						}
						if (!isOperator(peekToken(), OT_CURLY_BRACES_CLOSE))
							expectLineBreak();
					}
				}
				node->setLine(token->_line);
				return node;
			}
			case(OT_NAMESPACE):	{
				auto node = new AST::NamespaceDeclaration();
				node->setName(expectIdentifier());
				auto inner = parseInnerBlock();
				if (inner->isDeclaration());
				else if (inner->getStatementType() == ST_STATEMENT_BLOCK) 
					for (auto i : dynamic_cast<AST::StatementBlock*>(inner)->getStatements())
						if (!i->isDeclaration())
							throw DinoException("Expected a declaration", EXT_GENERAL, i->getLine());
				node->setStatement(inner);	
				node->setLine(token->_line);
				return node;
			}
			case(OT_DELETE):{
				auto op = new AST::UnaryOperationStatement();
				op->setOperator(((OperatorToken*)token)->_operator);
				op->setExpression(parseExpression());
				op->setLine(token->_line);
				return op;
			}
			case(OT_RETURN):{
			auto op = new AST::UnaryOperationStatement();
			op->setOperator(((OperatorToken*)token)->_operator);
			op->setExpression(parseOptionalExpression());
			op->setLine(token->_line);
			return op;
		}
		}
	}
	return NULL;
}

AST::Node * Parser::nud(Token * token)
{
	if (token->_type == TT_IDENTIFIER)
	{
		auto node = new AST::Identifier(token->_data);
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
			default: throw DinoException("Internal Lexer error", EXT_GENERAL, token->_line);
		}
		node->setLine(token->_line);
		return node;
	}
	if (isOperator(token, OT_PARENTHESIS_OPEN))
	{
		AST::Node* inner = parse();
		expectOperator(OT_PARENTHESIS_CLOSE);

		// Function literal
		if (peekToken()->_type == TT_IDENTIFIER || isOperator(peekToken(), OT_COLON) || isOperator(peekToken(), OT_CURLY_BRACES_OPEN))
		{
			auto params = convertToExpression(inner);
			switch (params->getExpressionType())	// make sure inner is a list of variable declarations
			{
				case(ET_LIST):
					for (auto i : dynamic_cast<AST::ExpressionList*>(inner)->getExpressions())
						if (i->getExpressionType() != ET_VARIABLE_DECLARATION)
							return inner;
				case(ET_VARIABLE_DECLARATION): break;
				default: return inner;
			}
			auto func = new AST::Function();
			func->addParameters(params);
			if (!isOperator(peekToken(), OT_COLON) && !isOperator(peekToken(), OT_CURLY_BRACES_OPEN))
				func->setReturnType(parseExpression());
			func->setContent(parseInnerBlock());
			func->setLine(token->_line);
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
			op->setExpression(parseExpression(leftPrecedence(ot, PREFIX)));
			return op;
		}

		auto op = new AST::UnaryOperation();
		op->setLine(token->_line);
		op->setOperator(ot->_operator);

		if (ot->_operator._type == OT_SQUARE_BRACKETS_OPEN)
		{
			if (eatOperator(OT_SQUARE_BRACKETS_CLOSE))
			{
				op->setExpression(NULL);
				return op;
			}
			op->setExpression(parseExpression());
			expectOperator(OT_SQUARE_BRACKETS_CLOSE);
			return op;
		}

		op->setExpression(parseExpression(leftPrecedence(ot, PREFIX)));
		return op;
	}
	throw DinoException("nud couldn't find an option", EXT_GENERAL, token->_line);
}

AST::Node * Parser::led(AST::Node * left, Token * token)
{
	if (token->_type == TT_IDENTIFIER)	// variable declaration
	{
		auto varDecl = new AST::VariableDeclaration();
		varDecl->setLine(token->_line);
		varDecl->setType(convertToExpression(left));
		varDecl->setVarId(token->_data);

		// Property declaration
		if (eatOperator(OT_COLON))
		{
			auto decl = new AST::PropertyDeclaration(varDecl);
			if (eatOperator(OT_GET))
				decl->setGet(parseInnerBlock());
			else if (eatOperator(OT_SET))
				decl->setSet(parseInnerBlock());
			decl->setLine(token->_line);
			return decl;
		}

		if (eatOperator(OT_CURLY_BRACES_OPEN))
		{
			auto decl = new AST::PropertyDeclaration(varDecl);
			if (eatOperator(OT_GET))
			{
				decl->setGet(parseInnerBlock());
				skipLineBreaks();
				if (eatOperator(OT_SET))
					decl->setSet(parseInnerBlock());
			}
			else if (eatOperator(OT_SET))
			{ 
				decl->setSet(parseInnerBlock());
				skipLineBreaks();
				if (eatOperator(OT_GET))
					decl->setGet(parseInnerBlock());
			}
			skipLineBreaks();
			expectOperator(OT_CURLY_BRACES_CLOSE);
			decl->setLine(token->_line);
			return decl;
		}
		// Function declaration
		if (eatOperator(OT_PARENTHESIS_OPEN))
		{
			auto decl = new AST::FunctionDeclaration(varDecl);
			decl->setLine(token->_line);
			decl->addParameter(parse());
			expectOperator(OT_PARENTHESIS_CLOSE);
			if (peekToken()->_type == TT_LINE_BREAK) decl->setContent(NULL);
			else decl->setContent(parseInnerBlock());
			return decl;
		}
		return varDecl;
	}
	if (isOperator(token, OT_IF))
	{
		auto node = new AST::ConditionalExpression();
		node->setThenBranch(convertToExpression(left));
		node->setCondition(parseExpression());
		skipLineBreaks();
		expectOperator(OT_ELSE);
		node->setElseBranch(parseExpression());
		node->setLine(token->_line);
		return node;
	}
	if (isOperator(token, OT_COMMA))
	{
		auto list = new AST::ExpressionList();
		list->addExpression(convertToExpression(left));
		list->addExpression(parseOptionalExpression(precedence(token, BINARY)));
		return list;
	}
	if (fromCategory(token, BINARY))
	{
		auto ot = ((OperatorToken*)token);

		if (ot->_operator._type == OT_PARENTHESIS_OPEN)
		{	
			auto funcCall = new AST::FunctionCall();
			funcCall->setFunctionId(convertToExpression(left));
			funcCall->setParameters(parseOptionalExpression());
			expectOperator(OT_PARENTHESIS_CLOSE);
			funcCall->setLine(token->_line);
			return funcCall;
		}
		if (OperatorsMap::isAssignment(ot->_operator._type))
		{
			auto op = new AST::Assignment();
			op->setOperator(ot->_operator);
			op->setLeft(convertToExpression(left));
			op->setRight(parseExpression(leftPrecedence(ot, BINARY)));
			op->setLine(token->_line);
			return op;
		}

		auto op = new AST::BinaryOperation();
		op->setLine(token->_line);
		op->setOperator(ot->_operator);
		op->setLeft(convertToExpression(left));
		if (ot->_operator._type == OT_SQUARE_BRACKETS_OPEN)
		{
			op->setRight(parseOptionalExpression());
			expectOperator(OT_SQUARE_BRACKETS_CLOSE);
		}
		else op->setRight(parseExpression(leftPrecedence(ot, BINARY)));
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
			op->setExpression(convertToExpression(left));
			return op;
		}
		auto op = new AST::UnaryOperation();
		op->setLine(token->_line);
		op->setIsPostfix(true);
		op->setOperator(((OperatorToken*)token)->_operator);
		op->setExpression(convertToExpression(left));
		return op;
	}
	throw DinoException("led could not find an option.", EXT_GENERAL, token->_line);
}
