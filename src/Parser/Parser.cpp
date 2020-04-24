#include "Parser.h"

int _lineNum;
int _index;

set<string> Parser::_parsedFiles;

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
	block->setPosition(peekToken()->_pos);
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
		throw ErrorReporter::report("expected a line break", ERR_PARSER, peekToken()->_pos);
}

void Parser::expectOperator(OperatorType ot)
{
	if (!eatOperator(ot)) 
		throw ErrorReporter::report("expected a '" + OperatorsMap::getOperatorByDefinition(ot).second._str.to_string() + "'", ERR_PARSER, peekToken()->_pos);
}

unicode_string Parser::expectIdentifier()
{
	if (peekToken()->_type == TT_IDENTIFIER)
		return nextToken()->_data;
	else throw ErrorReporter::report("expected an identifier.", ERR_PARSER, peekToken()->_pos);
}

AST::ExpressionList * Parser::expectIdentifierList()
{
	auto l = new AST::ExpressionList();
	l->setPosition(peekToken()->_pos);
	do
	{
		auto node = parseExpression(OperatorsMap::getOperatorByDefinition(OT_COMMA).second._binaryPrecedence);
		if (node && (node->getExpressionType() == ET_IDENTIFIER ||
			(node->getExpressionType() == ET_BINARY_OPERATION && ((AST::BinaryOperation*)node)->getOperator()._type == OT_PERIOD)))
			l->addExpression(node);
		else throw ErrorReporter::report("Expected an identifier or member access", ERR_PARSER, peekToken()->_pos);
	} while (eatOperator(OT_COMMA));
	return l;
}

AST::Identifier * Parser::convertToIdentifier(AST::Node * node, string errMsg)
{
	if (node != nullptr && node->isExpression() && dynamic_cast<AST::Expression*>(node)->getExpressionType() == ET_IDENTIFIER)
		return dynamic_cast<AST::Identifier*>(node);
	throw ErrorReporter::report(errMsg, ERR_PARSER, node->getPosition());
}

AST::Expression * Parser::convertToExpression(AST::Node * node)
{
	if (node == nullptr || node->isExpression())
		return dynamic_cast<AST::Expression*>(node);
	throw ErrorReporter::report("expected an expression", ERR_PARSER, node->getPosition());
}

AST::Statement * Parser::convertToStatement(AST::Node * node)
{
	if (node == nullptr || node->isStatement())
		return dynamic_cast<AST::Statement*>(node);
	throw ErrorReporter::report("expected a statement", ERR_PARSER, node->getPosition());
}

AST::StatementBlock * Parser::convertToStatementBlock(AST::Node * node)
{
	if (node == nullptr || node->isStatement())
	{
		auto stmnt = dynamic_cast<AST::Statement*>(node);
		if (stmnt->getStatementType() == ST_STATEMENT_BLOCK)
			return (AST::StatementBlock*)stmnt;
		auto bl = new AST::StatementBlock();
		bl->addStatement(stmnt);
		return bl;
	}
	throw ErrorReporter::report("expected a statement", ERR_PARSER, node->getPosition());
}

AST::Statement * Parser::parseStatement(int precedence)
{
	return convertToStatement(parse(precedence));
}

AST::Expression * Parser::parseExpression(int precedence)
{
	AST::Node* node = parse(precedence);
	if (node != nullptr && node->isExpression())
		return dynamic_cast<AST::Expression*>(node);
	throw ErrorReporter::report("expected an expression", ERR_PARSER, node ? node->getPosition() : peekToken()->_pos);
}

AST::Expression * Parser::parseOptionalExpression(int precedence)
{
	return convertToExpression(parse(precedence));
}

AST::StatementBlock * Parser::parseInnerBlock()
{
	if (eatOperator(OT_CURLY_BRACES_OPEN))
		return parseBlock(OT_CURLY_BRACES_CLOSE);
	else if (eatOperator(OT_COLON))
	{
		skipLineBreaks();
		auto *block = new AST::StatementBlock();
		block->addStatement(convertToStatement(parse()));
		return block;
	}
	throw ErrorReporter::report("expected a block statement.", ERR_PARSER, peekToken()->_pos);
}

#define fromCategory(tok, cat) (tok->_type == TT_OPERATOR && precedence(tok, cat) != NONE)

/*
	Returns the relevant operator precedence from the selected category(s) if token is an operator, otherwise returns NULL.
*/
int Parser::precedence(Token * token, int category)
{
	if (token->_type == TT_IDENTIFIER)
		return 135;
	if (token->_type != TT_OPERATOR)
		return 0;
	auto op = ((OperatorToken*)token)->_operator;
	switch (category) {
		case(BINARY):	return op._binaryPrecedence;
		case(PREFIX):	return op._prefixPrecedence;
		case(POSTFIX):	return op._postfixPrecedence;
		case(BINARY | POSTFIX):	return op._binaryPrecedence != NONE ? op._binaryPrecedence : op._postfixPrecedence;
		default:		return 0;
	}
	
}

int Parser::leftPrecedence(OperatorToken * token, int category)
{
	int prec = precedence(token, category);
	if (token->_operator._associativity == RIGHT_TO_LEFT) prec--;
	return prec;
}


AST::StatementBlock * Parser::parseFile(string fileName, bool showLexerOutput)
{
	if (_parsedFiles.count(fileName))
		return new AST::StatementBlock();
	_parsedFiles.insert(fileName);
	std::ifstream t;
	try { t = std::ifstream(fileName); }
	catch (exception e) { std::cout << e.what() << std::endl; exit(0); }
	std::stringstream buffer;
	buffer << t.rdbuf();
	std::string str = buffer.str();
	auto vec = Lexer::lex(str, fileName);
	// std::cout << "File: " << fileName << std::endl;
	// auto vec = Preprocessor::preprocess(lexed);
	if (showLexerOutput)
		for (auto i : vec) 
			printToken(i);
	Parser p = Parser(vec);
	auto block = p.parseBlock();
	// block->setFile(fileName);
	return block;
}



AST::StatementBlock * Parser::importFile(PositionInfo currPos)
{
	auto t = nextToken();
	if (t->_type == TT_LITERAL && ((LiteralToken<bool>*)t)->_literalType != LT_STRING)
		throw ErrorReporter::report("Expected a file path", ERR_PARSER, currPos);

	string importPath = ((LiteralToken<string>*)t)->_value;
	auto dir = opendir(importPath.c_str());
	if (!dir)
		throw ErrorReporter::report("Could not open directory \"" + importPath + '\"', ERR_PARSER, currPos);
	AST::StatementBlock *block = NULL;
	while (auto ent = readdir(dir))
	{
		string fileName(ent->d_name);
		if (fileName.substr(fileName.find_last_of(".")) == ".dinh")
		{
			if (block)
				block->addStatement(parseFile(importPath + '/' + fileName));
			else block = parseFile(importPath + '/' + fileName);
		}
	}
	closedir(dir);
	if (!block)
		throw ErrorReporter::report("No .dinh files in directory \"" + importPath + '\"', ERR_PARSER, currPos);
	auto import = new AST::Import(importPath);
	import->setPosition(currPos);
	block->addStatement(import);
	return block;
}

AST::StatementBlock * Parser::includeFile()
{
	auto t = nextToken();
	if (t->_type == TT_LITERAL && ((LiteralToken<bool>*)t)->_literalType == LT_STRING)
	{
		string fileName = ((LiteralToken<string>*)t)->_value;
		return parseFile(fileName);
	}
	else throw ErrorReporter::report("Expected a file path", ERR_PARSER, peekToken()->_pos);
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
		&& (/*peekToken()->_type == TT_IDENTIFIER || */precedence(peekToken(), BINARY | POSTFIX) > lastPrecedence))
		left = led(left, nextToken());
	return left;
}

AST::Node * Parser::std(Token * token)
{
	if (token->_type == TT_OPERATOR && OperatorsMap::isKeyword(((OperatorToken*)token)->_operator))
	{
		switch (((OperatorToken*)token)->_operator._type)	
		{
			case(OT_IMPORT): {
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
			case(OT_INCLUDE): {
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
			case(OT_CONST): {
				auto node = new AST::ConstDeclaration();
				node->setName(expectIdentifier());
				expectOperator(OT_ASSIGN_EQUAL);
				node->setExpression(parseExpression());
				node->setPosition(token->_pos);
				return node;
			}
			case(OT_WHILE): {
				auto node = new AST::WhileLoop();
				node->setCondition(parseExpression());
				node->setStatement(parseInnerBlock());
				node->setPosition(token->_pos);
				return node;
			}
			case(OT_FOR): {
				auto node = new AST::ForLoop();
				node->setBegin(parseStatement());
				expectLineBreak();
				node->setCondition(parseExpression());
				expectLineBreak();
				node->setIncrement(parseStatement());
				if (peekToken()->_type == TT_LINE_BREAK)
				{
					node->setStatement(new AST::StatementBlock());
					node->getStatement()->setPosition(peekToken()->_pos);
				}
				else node->setStatement(parseInnerBlock());				
				node->setPosition(token->_pos);
				return node;
			}
			case(OT_DO): {
				auto node = new AST::DoWhileLoop();
				node->setStatement(parseInnerBlock());
				skipLineBreaks();
				expectOperator(OT_WHILE);
				node->setCondition(parseExpression());
				node->setPosition(token->_pos);
				return node;
			}
			case(OT_IF): {
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
				if (eatOperator(OT_ELSE)) {
					if (isOperator(peekToken(), OT_IF)) {
						auto block = new AST::StatementBlock();
						block->addStatement(parseStatement());
						node->setElseBranch(block);
					}
					else node->setElseBranch(parseInnerBlock());
				}
				else 
				{
					node->setElseBranch(NULL);
					if (b) _index--;
				}
				node->setPosition(token->_pos);
				return node;
			}
			case(OT_SWITCH): {
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
			case(OT_UNLESS): {
				auto node = new AST::IfThenElse();
				node->setCondition(parseExpression());
				node->setElseBranch(parseInnerBlock());
				node->setThenBranch(NULL);
				node->setPosition(token->_pos);
				return node;
			}
			case(OT_TRY): {
				auto node = new AST::TryCatch();
				node->setTryBlock(parseInnerBlock());
				expectOperator(OT_CATCH);
				node->setCatchBlock(parseInnerBlock());
				node->setPosition(token->_pos);
				return node;
			}
			case(OT_CATCH): throw ErrorReporter::report("Missing \"try\" statement before \"catch\"", ERR_PARSER, token->_pos);
			case(OT_ELSE): throw ErrorReporter::report("Missing \"if\" statement before \"else\"", ERR_PARSER, token->_pos);
			case(OT_TYPE): {
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
			case(OT_INTERFACE):	{
				auto node = new AST::InterfaceDeclaration();
				node->setName(expectIdentifier());
				if (eatOperator(OT_IS))
					node->setImplements(expectIdentifierList());
				if (eatOperator(OT_COLON))
				{
					auto decl = parseStatement();
					switch (decl->getStatementType())
					{
					case(ST_PROPERTY_DECLARATION): node->addProperty(dynamic_cast<AST::PropertyDeclaration*>(decl)); break;
					case(ST_FUNCTION_DECLARATION): node->addFunction(dynamic_cast<AST::FunctionDeclaration*>(decl)); break;
					default: throw ErrorReporter::report("interfaces may only contain properties and functions", ERR_PARSER, decl->getPosition());
					}
				}
				else {
					expectOperator(OT_CURLY_BRACES_OPEN);
					while (!eatOperator(OT_CURLY_BRACES_CLOSE))
					{
						auto decl = parseStatement();
						switch (decl->getStatementType())
						{
						case(ST_PROPERTY_DECLARATION): node->addProperty(dynamic_cast<AST::PropertyDeclaration*>(decl)); break;
						case(ST_FUNCTION_DECLARATION): node->addFunction(dynamic_cast<AST::FunctionDeclaration*>(decl)); break;
						default: throw ErrorReporter::report("interfaces may only contain properties and functions", ERR_PARSER, decl->getPosition());
						}
						if (!isOperator(peekToken(), OT_CURLY_BRACES_CLOSE))
							expectLineBreak();
					}
				}
				node->setPosition(token->_pos);
				return node;
			}
			case(OT_NAMESPACE):	{
				auto name = expectIdentifier();
				AST::NamespaceDeclaration *node;
				auto inner = parseInnerBlock();

				if(_namespaces.count(name) == 0)
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
			case(OT_BREAK): case(OT_CONTINUE): {
				auto op = new AST::UnaryOperationStatement();
				op->setOperator(((OperatorToken*)token)->_operator);
				op->setPosition(token->_pos);
				return op;
			}
			case(OT_DELETE): {
				auto op = new AST::UnaryOperationStatement();
				op->setOperator(((OperatorToken*)token)->_operator);
				op->setExpression(parseExpression());
				op->setPosition(token->_pos);
				return op;
			}
			case(OT_RETURN): case(OT_EXTERN): case(OT_THROW): {
				auto op = new AST::UnaryOperationStatement();
				op->setOperator(((OperatorToken*)token)->_operator);
				op->setExpression(parseOptionalExpression());
				op->setPosition(token->_pos);
				return op;
			}
			default: 
				return NULL;
		}
	}
	return NULL;
}

AST::Node * Parser::nud(Token * token)
{
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

		// Function literal
		if (peekToken()->_type == TT_IDENTIFIER || isOperator(peekToken(), OT_COLON) || isOperator(peekToken(), OT_CURLY_BRACES_OPEN))
		{
			auto params = convertToExpression(inner);
			if (params != nullptr)
			{
				switch (params->getExpressionType())	// make sure inner is a list of variable declarations
				{
				case(ET_LIST):
					for (auto i : dynamic_cast<AST::ExpressionList*>(inner)->getExpressions())
						if (i->getExpressionType() != ET_VARIABLE_DECLARATION)
							return inner;
				case(ET_VARIABLE_DECLARATION): break;
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

AST::Node * Parser::led(AST::Node * left, Token * token)
{
	if (token->_type == TT_IDENTIFIER)	// variable declaration
	{
		auto varDecl = new AST::VariableDeclaration();
		varDecl->setPosition(token->_pos);
		varDecl->getPosition().startPos = left->getPosition().startPos;
		varDecl->setType(convertToExpression(left));
		varDecl->setVarId(token->_data);

		// Property declaration
		if (eatOperator(OT_COLON))
		{
			auto decl = new AST::PropertyDeclaration(varDecl);

			#define PARSE_INNER_IF_BLOCK (isOperator(peekToken(), OT_COLON) || isOperator(peekToken(), OT_CURLY_BRACES_OPEN) ? parseInnerBlock() : NULL)

			if (eatOperator(OT_GET))
				decl->setGet(PARSE_INNER_IF_BLOCK);
			else if (eatOperator(OT_SET))
				decl->setSet(PARSE_INNER_IF_BLOCK);
			decl->setPosition(token->_pos);
			return decl;
		}

		if (eatOperator(OT_CURLY_BRACES_OPEN))
		{
			auto decl = new AST::PropertyDeclaration(varDecl);
			if (eatOperator(OT_GET))
			{
				decl->setGet(PARSE_INNER_IF_BLOCK);
				skipLineBreaks();
				if (eatOperator(OT_SET))
					decl->setSet(PARSE_INNER_IF_BLOCK);
			}
			else if (eatOperator(OT_SET))
			{ 
				decl->setSet(PARSE_INNER_IF_BLOCK);
				skipLineBreaks();
				if (eatOperator(OT_GET))
					decl->setGet(PARSE_INNER_IF_BLOCK);
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
			decl->addParameter(parse());
			expectOperator(OT_PARENTHESIS_CLOSE);
			if (peekToken()->_type == TT_LINE_BREAK || isOperator(peekToken(), OT_CURLY_BRACES_CLOSE)) 
				decl->setContent(NULL);
			else decl->setContent(parseInnerBlock());
			return decl;
		}
		return varDecl;
	}
	// if (isOperator(token, OT_IF))
	// {
	// 	auto node = new AST::ConditionalExpression();
	// 	node->setThenBranch(convertToExpression(left));
	// 	node->setCondition(parseExpression());
	// 	skipLineBreaks();
	// 	expectOperator(OT_ELSE);
	// 	node->setElseBranch(parseExpression());
	// 	node->setPosition(token->_pos);
	// 	return node;
	// }
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
			if (eatOperator(OT_THEN))
				if (isOperator(peekToken(), OT_COLON) || isOperator(peekToken(), OT_CURLY_BRACES_OPEN))
					node->setThenBranch(parseInnerBlock());
				else node->setThenBranch(convertToStatementBlock(parseStatement()));
			else node->setThenBranch(NULL);
			node->setPosition(token->_pos);
			return node;
		}
		if (ot->_operator._type == OT_IF)
		{
			auto node = new AST::IfThenElse();
			node->setCondition(parseExpression());
			node->setThenBranch(convertToStatementBlock(left));
			if (eatOperator(OT_ELSE))
				if (isOperator(peekToken(), OT_COLON) || isOperator(peekToken(), OT_CURLY_BRACES_OPEN))
					node->setElseBranch(parseInnerBlock());
				else node->setElseBranch(convertToStatementBlock(parseStatement()));
			else node->setElseBranch(NULL);
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
