/*
	This file defines the four core functions of a pratt-parser: 
		* parse
		* std
		* nud
		* led.
*/
#include "Parser.h"

#define fromCategory(tok, cat) (tok->_type == TT_OPERATOR && precedence(tok, cat) != NONE)

AST::Node * Parser::parse(int lastPrecedence, bool isExpression)
{
	if (!isExpression)
		if (auto left = std())
			return left;
	auto left = nud();
	if (!left) return NULL;
	while (precedence(peekToken(), BINARY | POSTFIX) > lastPrecedence)
		left = led(left);
	return left;
}

/* Standard-denotation */
AST::Node * Parser::std()
{
	Token* token = nextToken();
	if (token->_type == TT_OPERATOR && OperatorsMap::isKeyword(((OperatorToken*)token)->_operator))
	{
		switch (((OperatorToken*)token)->_operator._type)	
		{
			case OT_IMPORT: {
				if (eatOperator(OT_PARENTHESIS_OPEN))
				{
					auto block = includeFile();
					eatLineBreak();
					while (!eatOperator(OT_PARENTHESIS_CLOSE))
					{
						block->addStatement(importFile(token->_pos));
						eatLineBreak();
					}
					return block;
				}
				else return importFile(token->_pos);
			}
			case OT_INCLUDE: {
				if (eatOperator(OT_PARENTHESIS_OPEN))
				{
					auto block = includeFile();
					eatLineBreak();
					while (!eatOperator(OT_PARENTHESIS_CLOSE))
					{
						block->addStatement(includeFile());
						eatLineBreak();
					}
					return block;
				}
				else return includeFile();
			}
			case OT_CONST: {
				auto node = new AST::ConstDeclaration();
				node->setName(expectIdentifier());
				expectOperator(OT_ASSIGN_EQUAL);
				node->setExpression(parseExpression());
				node->setPosition(token->_pos);
				return node;
			}
			case OT_WHILE: {
				auto node = new AST::WhileLoop();
				node->setCondition(parseExpression());
				node->setStatement(parseInnerBlock());
				node->setPosition(token->_pos);
				return node;
			}
			case OT_FOR: {
				auto node = new AST::ForLoop();
				node->setBegin(parseOptionalStatement());
				expectLineBreak();
				node->setCondition(parseExpression());
				expectLineBreak();
				node->setIncrement(parseOptionalStatement());
				node->setStatement(parseInnerBlock());
				node->setPosition(token->_pos);
				return node;
			}
			case OT_DO: {
				auto node = new AST::DoWhileLoop();
				node->setStatement(parseInnerBlock());
				skipLineBreaks();
				expectOperator(OT_WHILE);
				node->setCondition(parseExpression());
				node->setPosition(token->_pos);
				return node;
			}
			case OT_IF: {
				auto cond = parseExpression();
				if (eatOperator(OT_THEN)) {	// conditional expression
					auto node = new AST::ConditionalExpression();
					node->setCondition(cond);
					node->setThenBranch(parseExpression());
					skipLineBreaks();
					expectOperator(OT_ELSE);
					node->setElseBranch(parseExpression());
					node->setPosition(token->_pos);
					return node;
				}


				auto node = new AST::IfThenElse();
				node->setCondition(cond);
				node->setThenBranch(parseInnerBlock());
				bool b = eatLineBreak();
				if (eatOperator(OT_ELSE))
					node->setElseBranch(parseInnerBlock());
				else 
				{
					node->setElseBranch(NULL);
					if (b) _index--;
				}
				node->setPosition(token->_pos);
				return node;
			}
			case OT_SWITCH: {
				auto node = new AST::SwitchCase();
				if (!eatOperator(OT_CURLY_BRACES_OPEN))
				{
					node->setExpression(parseExpression());
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
					else throw ErrorReporter::report("expected 'case' or 'default'", ERR_PARSER, peekToken()->_pos);

					if (!isOperator(peekToken(), OT_CURLY_BRACES_CLOSE))
						expectLineBreak();
				}
				node->setPosition(token->_pos);
				return node;
			}
			case OT_UNLESS: {
				auto node = new AST::IfThenElse();
				node->setCondition(parseExpression());
				node->setElseBranch(parseInnerBlock());
				node->setThenBranch(NULL);
				node->setPosition(token->_pos);
				return node;
			}
			case OT_TRY: {
				auto node = new AST::TryCatch();
				node->setTryBlock(parseInnerBlock());
				expectOperator(OT_CATCH);
				node->setCatchBlock(parseInnerBlock());
				node->setPosition(token->_pos);
				return node;
			}
			case OT_CATCH: throw ErrorReporter::report("Missing \"try\" statement before \"catch\"", ERR_PARSER, token->_pos);
			case OT_ELSE: throw ErrorReporter::report("Missing \"if\" statement before \"else\"", ERR_PARSER, token->_pos);
			case OT_ENUM: {
				auto node = new AST::EnumDeclaration(expectIdentifier());
				if (eatOperator(OT_IS)) node->setType(parseExpression());
				expectOperator(OT_CURLY_BRACES_OPEN);
				while (eatLineBreak(), !eatOperator(OT_CURLY_BRACES_CLOSE))
					node->addMember(expectIdentifier(), convertToLiteral(parseOptionalExpression(), "explicit enum value must be a literal"));
				node->setPosition(token->_pos);
				return node;
			}
			case OT_TYPE: {
				auto node = new AST::TypeDeclaration();
				node->setName(expectIdentifier());
				if (eatOperator(OT_IS))
					node->setInterfaces(expectIdentifierList());
				expectOperator(OT_CURLY_BRACES_OPEN);
				while (!eatOperator(OT_CURLY_BRACES_CLOSE))
				{
					auto decl = parseStatement();
					switch (decl->getStatementType())
					{
					case(ST_VARIABLE_DECLARATION): node->addVariableDeclaration(dynamic_cast<AST::VariableDeclaration*>(decl)); break;
					case(ST_FUNCTION_DECLARATION): 
						if (!dynamic_cast<AST::FunctionDeclaration*>(decl)->getContent())
							throw ErrorReporter::report("missing function body", ERR_PARSER, decl->getPosition());
						node->addFunctionDeclaration(dynamic_cast<AST::FunctionDeclaration*>(decl)); break;
					case(ST_PROPERTY_DECLARATION): 
						if (!dynamic_cast<AST::PropertyDeclaration*>(decl)->getGet() && !dynamic_cast<AST::PropertyDeclaration*>(decl)->getSet())
							throw ErrorReporter::report("missing propery body", ERR_PARSER, decl->getPosition());
						node->addPropertyDeclaration(dynamic_cast<AST::PropertyDeclaration*>(decl)); break;
					default: throw ErrorReporter::report("expected a variable, property or function declaration", ERR_PARSER, decl->getPosition());
					}
					skipLineBreaks();
				}
				node->setPosition(token->_pos);
				return node;
			}
			case OT_INTERFACE:	{
				auto node = new AST::InterfaceDeclaration();
				node->setName(expectIdentifier());
				_inInterface = true;
				if (eatOperator(OT_IS))
					node->setImplements(expectIdentifierList());
				if (eatOperator(OT_CURLY_BRACES_OPEN))
				{
					while (!eatOperator(OT_CURLY_BRACES_CLOSE))
					{
						auto decl = parseStatement();
						switch (decl->getStatementType())
						{
							case ST_PROPERTY_DECLARATION: node->addProperty(((AST::PropertyDeclaration*)decl)); break;
							case ST_FUNCTION_DECLARATION: node->addFunction(((AST::FunctionDeclaration*)decl)); break;
							default: throw ErrorReporter::report("interfaces may only contain properties and functions", ERR_PARSER, decl->getPosition());
						}
						if (!isOperator(peekToken(), OT_CURLY_BRACES_CLOSE))
							expectLineBreak();
					}
				}
				else
				{
					eatOperator(OT_COLON);
					auto decl = parseStatement();
					switch (decl->getStatementType())
					{
						case ST_PROPERTY_DECLARATION: node->addProperty(((AST::PropertyDeclaration*)decl)); break;
						case ST_FUNCTION_DECLARATION: node->addFunction(((AST::FunctionDeclaration*)decl)); break;
						default: throw ErrorReporter::report("interfaces may only contain properties and functions", ERR_PARSER, decl->getPosition());
					}
				}
				node->setPosition(token->_pos);
				_inInterface = false;
				return node;
			}
			case OT_NAMESPACE:	{
				auto name = expectIdentifier();
				AST::NamespaceDeclaration *node;
				auto inner = parseInnerBlock();

				if (_namespaces.count(name) == 0)
				{	
					node = new AST::NamespaceDeclaration();
					node->setName(name); 
					for (auto i : dynamic_cast<AST::StatementBlock*>(inner)->getStatements())
						if (!i->isDeclaration())
							throw ErrorReporter::report("Expected a declaration", ERR_PARSER, i->getPosition());
					node->setStatement(inner);
					node->setPosition(token->_pos);
					_namespaces[name] = node;
				}
				else
				{
					node = _namespaces[name];
					for (auto i : dynamic_cast<AST::StatementBlock*>(inner)->getStatements())
					{
						if (!i->isDeclaration())
							throw ErrorReporter::report("Expected a declaration", ERR_PARSER, i->getPosition());
						node->getStatement()->addStatement(i);
					}
				}
				return node;
			}
			case OT_BREAK: case OT_CONTINUE: {
				auto op = new AST::UnaryOperationStatement();
				op->setOperator(((OperatorToken*)token)->_operator);
				op->setPosition(token->_pos);
				return op;
			}
			case OT_DELETE: {
				auto op = new AST::UnaryOperationStatement();
				op->setOperator(((OperatorToken*)token)->_operator);
				op->setExpression(parseExpression());
				op->setPosition(token->_pos);
				return op;
			}
			case OT_RETURN: case OT_EXTERN: case OT_THROW: {
				auto op = new AST::UnaryOperationStatement();
				op->setOperator(((OperatorToken*)token)->_operator);
				op->setExpression(parseOptionalExpression());
				op->setPosition(token->_pos);
				return op;
			}
			default: 
				_index--;
				return NULL;
		}
	}
	_index--;
	return NULL;
}

/* Null-denotation */
AST::Node * Parser::nud()
{
	if (   peekToken()->_type == TT_LINE_BREAK		     || isOperator(peekToken(), OT_SQUARE_BRACKETS_CLOSE)
		|| isOperator(peekToken(), OT_PARENTHESIS_CLOSE) || isOperator(peekToken(), OT_CURLY_BRACES_CLOSE) 
		|| isOperator(peekToken(), OT_EOF)				 || isOperator(peekToken(), OT_COLON))
		return NULL;
	Token* token = nextToken();
	if (token->_type == TT_IDENTIFIER)
	{
		auto node = new AST::Identifier(token->_data);
		node->setPosition(token->_pos);
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
			case (LT_CHARACTER): node = new AST::Character(((LiteralToken<unicode_char>*)token)->_value); break;
			case (LT_FRACTION): node = new AST::Fraction(((LiteralToken<float>*)token)->_value); break;
			case (LT_NULL): node = new AST::Null(); break;
			default: throw ErrorReporter::report("Internal Lexer error", ERR_PARSER, token->_pos);
		}
		node->setPosition(token->_pos);
		return node;
	}
	if (isOperator(token, OT_PARENTHESIS_OPEN))
	{
		AST::Node* inner = parse();
		expectOperator(OT_PARENTHESIS_CLOSE);

		// Function literal (FIXME!)
		if (peekToken()->_type == TT_IDENTIFIER || isOperator(peekToken(), OT_COLON) || isOperator(peekToken(), OT_CURLY_BRACES_OPEN))
		{
			auto params = convertToExpression(inner);
			if (params != nullptr)
			{
				switch (params->getExpressionType())	// make sure inner is a list of variable declarations
				{
				case ET_LIST:
					for (auto i : dynamic_cast<AST::ExpressionList*>(inner)->getExpressions())
						if (i->getExpressionType() != ET_VARIABLE_DECLARATION)
							return inner;
				case ET_VARIABLE_DECLARATION: break;
				default: return inner;
				}
			}
			auto func = new AST::Function();
			func->addParameters(params);
			if (!isOperator(peekToken(), OT_COLON) && !isOperator(peekToken(), OT_CURLY_BRACES_OPEN))
				func->setReturnType(parseExpression());
			func->setContent(parseInnerBlock());
			func->setPosition(token->_pos);
			return func;
		}
		return inner;
	}
	if (isOperator(token, OT_IF))
	{
		// conditional expression
		auto cond = parseExpression();
		expectOperator(OT_THEN);
		auto node = new AST::ConditionalExpression();
		node->setCondition(cond);
		node->setThenBranch(parseExpression());
		skipLineBreaks();
		expectOperator(OT_ELSE);
		node->setElseBranch(parseExpression());
		node->setPosition(token->_pos);
		return node;
	}
	if (fromCategory(token, PREFIX))
	{
		auto ot = ((OperatorToken*)token);
		if (isOperator(token, OT_INCREMENT) || isOperator(token, OT_DECREMENT))
		{
			auto op = new AST::Increment(isOperator(token, OT_INCREMENT));
			op->setPosition(token->_pos);
			op->setOperator(ot->_operator);
			op->setExpression(parseExpression(leftPrecedence(ot, PREFIX)));
			return op;
		}

		auto op = new AST::UnaryOperation();
		op->setPosition(token->_pos);
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
	throw ErrorReporter::report("unexpected token \"" + token->_data.to_string() + "\"", ERR_PARSER, token->_pos);
}

/* Left-denotation */
AST::Node * Parser::led(AST::Node * left)
{
	Token* token = nextToken();
	if (token->_type == TT_IDENTIFIER)	// variable declaration
	{
		auto varDecl = new AST::VariableDeclaration();
		varDecl->setPosition(token->_pos);
		varDecl->getPosition().startPos = left->getPosition().startPos;
		varDecl->setType(convertToExpression(left));
		varDecl->setVarId(token->_data);

		// Property declaration
		if (eatOperator(OT_COLON) || isOperator(peekToken(), OT_GET) || isOperator(peekToken(), OT_SET))
		{
			auto decl = new AST::PropertyDeclaration(varDecl);

			if (eatOperator(OT_GET))
				decl->setGet(parseInnerBlock());
			else if (eatOperator(OT_SET))
				decl->setSet(parseInnerBlock());
			else throw ErrorReporter::report("expected \"get\" or \"set\"", ERR_PARSER, token->_pos);
			decl->setPosition(token->_pos);
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
			decl->setPosition(token->_pos);
			return decl;
		}
		// Function declaration
		if (eatOperator(OT_PARENTHESIS_OPEN))
		{
			auto decl = new AST::FunctionDeclaration(varDecl);
			decl->setPosition(token->_pos);
			decl->addParameter(parse(0, true));
			expectOperator(OT_PARENTHESIS_CLOSE);
			if (_inInterface && (peekToken()->_type == TT_LINE_BREAK || isOperator(peekToken(), OT_CURLY_BRACES_CLOSE))) 
				decl->setContent(NULL);
			else decl->setContent(parseInnerBlock());
			return decl;
		}

		// Generic function declaration
		if (eatOperator(OT_SQUARE_BRACKETS_OPEN))
		{
			auto decl = new AST::FunctionDeclaration(varDecl);
			decl->setGenerics(parseOptionalExpression());
			expectOperator(OT_SQUARE_BRACKETS_CLOSE);
			expectOperator(OT_PARENTHESIS_OPEN);
			decl->setPosition(token->_pos);
			decl->addParameter(parse(0, false));
			expectOperator(OT_PARENTHESIS_CLOSE);
			if (peekToken()->_type == TT_LINE_BREAK || isOperator(peekToken(), OT_CURLY_BRACES_CLOSE)) 
				decl->setContent(NULL);
			else decl->setContent(parseInnerBlock());
			return decl;
		}
		return varDecl;
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
			funcCall->setArguments(parseOptionalExpression());
			expectOperator(OT_PARENTHESIS_CLOSE);
			funcCall->setPosition(token->_pos);
			return funcCall;
		}
		if (ot->_operator._type == OT_UNLESS)
		{
			auto node = new AST::IfThenElse();
			node->setCondition(parseExpression());
			node->setElseBranch(convertToStatementBlock(left));
			node->setThenBranch(eatOperator(OT_THEN) ? parseInnerBlock() : NULL);
			node->setPosition(token->_pos);
			return node;
		}
		if (ot->_operator._type == OT_IF)
		{
			auto node = new AST::IfThenElse();
			node->setCondition(parseExpression());
			node->setThenBranch(convertToStatementBlock(left));
			node->setElseBranch(eatOperator(OT_ELSE) ? parseInnerBlock() : NULL);
			node->setPosition(token->_pos);
			return node;
		}
		if (OperatorsMap::isAssignment(ot->_operator._type))
		{
			if (ot->_operator._type == OT_SHORT_VAR_DECL)
			{
				auto ident = convertToIdentifier(left, "left of short variable declaration must be an identifier");
				auto varDecl = new AST::VariableDeclaration();
				varDecl->setType(new AST::Identifier(unicode_string("var")));
				varDecl->setVarId(ident->getVarId());

				auto op = new AST::Assignment();
				op->setOperator(OperatorsMap::getOperatorByDefinition(OT_ASSIGN_EQUAL).second);
				op->setLeft(varDecl);
				op->setRight(parseExpression(leftPrecedence(ot, BINARY)));
				op->setPosition(token->_pos);
				return op;
			}
			

			auto op = new AST::Assignment();
			op->setOperator(ot->_operator);
			op->setLeft(convertToExpression(left));
			op->setRight(parseExpression(leftPrecedence(ot, BINARY)));
			op->setPosition(token->_pos);
			return op;
		}
		if (OperatorsMap::isComparison(ot->_operator._type))
		{
			auto comp = new AST::Comparison();
			comp->setPosition(token->_pos);
			comp->addExpression(convertToExpression(left));
			comp->addOperator(ot->_operator);
			comp->addExpression(parseExpression(leftPrecedence(ot, BINARY)));
			while (peekToken()->_type == TT_OPERATOR && OperatorsMap::isComparison(((OperatorToken*)peekToken())->_operator._type))
			{
				comp->addOperator(((OperatorToken*)nextToken())->_operator);
				comp->addExpression(parseExpression(leftPrecedence(ot, BINARY)));
			}
			return comp;
		}
		auto op = new AST::BinaryOperation();
		op->setPosition(token->_pos);
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
			auto op = new AST::Increment(isOperator(token, OT_INCREMENT));
			op->setPosition(token->_pos);
			op->setOperator(((OperatorToken*)token)->_operator);
			op->setExpression(convertToExpression(left));
			return op;
		}
		auto op = new AST::UnaryOperation();
		op->setPosition(token->_pos);
		op->setIsPostfix(true);
		op->setOperator(((OperatorToken*)token)->_operator);
		op->setExpression(convertToExpression(left));
		return op;
	}
	throw ErrorReporter::report("unexpected token \"" + token->_data.to_string() + "\"", ERR_PARSER, token->_pos);
}
