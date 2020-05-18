/*
    Utility parser functions.
*/
#include "Parser.h"

Token * Parser::getToken(uint index)
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
		throw ErrorReporter::report("expected a line break", ERR_GENERAL, peekToken()->_pos);
}

void Parser::expectOperator(OperatorType ot)
{
	if (!eatOperator(ot)) 
		throw ErrorReporter::report(
			"expected `" + OperatorsMap::getOperatorByDefinition(ot).second._str.to_string() + "`, found `" + peekToken()->_data.to_string() + "`",
			"expected `" + OperatorsMap::getOperatorByDefinition(ot).second._str.to_string() + "`",
			ERR_GENERAL, peekToken()->_pos
		);
}

unicode_string Parser::expectIdentifier()
{
	if (peekToken()->_type == TT_IDENTIFIER)
		return nextToken()->_data;
	else throw ErrorReporter::report("expected an identifier", ERR_GENERAL, peekToken()->_pos);
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
		else throw ErrorReporter::report("Expected an identifier or member access", ERR_GENERAL, peekToken()->_pos);
	} while (eatOperator(OT_COMMA));
	return l;
}

AST::Identifier * Parser::convertToIdentifier(AST::Node * node, string errMsg)
{
	if (node != nullptr && node->isExpression() && dynamic_cast<AST::Expression*>(node)->getExpressionType() == ET_IDENTIFIER)
		return dynamic_cast<AST::Identifier*>(node);
	throw ErrorReporter::report(errMsg, ERR_GENERAL, node->getPosition());
}

AST::Expression * Parser::convertToExpression(AST::Node * node)
{
	if (node == nullptr || node->isExpression())
		return dynamic_cast<AST::Expression*>(node);
	throw ErrorReporter::report("expected an expression", ERR_GENERAL, node->getPosition());
}

AST::Statement * Parser::convertToStatement(AST::Node * node)
{
	if (node == nullptr || node->isStatement())
		return dynamic_cast<AST::Statement*>(node);
	if (node->isExpression() && dynamic_cast<AST::Expression*>(node)->getExpressionType() == ET_COMPARISON && 
		dynamic_cast<AST::Comparison*>(node)->getOperators()[0]._type == OT_EQUAL)
		throw ErrorReporter::report(
			"expected a statement", 
			"expected a statement\n" +
			BOLD(FBLK("help: the `=` operator is used for comparisons\n")) +
			BOLD(FBLK("did you mean `≡`?")), 
			ERR_GENERAL, node->getPosition()
		);
	throw ErrorReporter::report("expected a statement", ERR_GENERAL, node->getPosition());
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
	if (node->isExpression() && dynamic_cast<AST::Expression*>(node)->getExpressionType() == ET_COMPARISON && 
		dynamic_cast<AST::Comparison*>(node)->getOperators()[0]._type == OT_EQUAL)
		throw ErrorReporter::report(
			"expected a statement", 
			"expected a statement\nhelp: the `=` operator is used for comparisons\ndid you mean `≡`?", 
			ERR_GENERAL, node->getPosition()
		);
	throw ErrorReporter::report("expected a statement", ERR_GENERAL, node->getPosition());
}

AST::Statement * Parser::parseStatement(int precedence)
{
	AST::Node* node = parse(precedence);
	if (node != nullptr && node->isStatement())
		return dynamic_cast<AST::Statement*>(node);
	if (node->isExpression() && dynamic_cast<AST::Expression*>(node)->getExpressionType() == ET_COMPARISON && 
		dynamic_cast<AST::Comparison*>(node)->getOperators()[0]._type == OT_EQUAL)
		throw ErrorReporter::report(
			"expected a statement", 
			"expected a statement\nhelp: the `=` operator is used for comparisons\ndid you mean `≡`?", 
			ERR_GENERAL, node ? node->getPosition() : peekToken()->_pos
		);
	throw ErrorReporter::report("expected a statement", ERR_GENERAL, node ? node->getPosition() : peekToken()->_pos);
}

AST::Statement * Parser::parseOptionalStatement(int precedence)
{
	return convertToStatement(parse(precedence));
}

AST::Expression * Parser::parseExpression(int precedence)
{
	AST::Node* node = parse(precedence, true);
	if (node != nullptr && node->isExpression())
		return dynamic_cast<AST::Expression*>(node);
	throw ErrorReporter::report("expected an expression", ERR_GENERAL, node ? node->getPosition() : peekToken()->_pos);
}

AST::Expression * Parser::parseOptionalExpression(int precedence)
{
	return convertToExpression(parse(precedence, true));
}

void Parser::assertIsDeclaration(AST::Statement * node)
{
	if (!node->isDeclaration())
		throw ErrorReporter::report("Expected a declaration", ERR_GENERAL, node->getPosition());
}

AST::StatementBlock * Parser::parseInnerBlock()
{
	if (_inInterface)
		return NULL;
	if (eatOperator(OT_CURLY_BRACES_OPEN))
		return parseBlock(OT_CURLY_BRACES_CLOSE);
	else
	{
		auto pos = peekToken()->_pos;
		eatOperator(OT_COLON);
		eatLineBreak();
		if (isOperator(peekToken(), OT_CURLY_BRACES_OPEN))
			throw ErrorReporter::report(ErrorReporter::Error(
				"unexpected `{`", 
				"dangling curly braces\n" +
				BOLD(FBLK("help: curly braces must be opened at end of line\n")) +
				BOLD(FBLK("try moving it to the end of the previous line")),
				ERR_GENERAL, 
				peekToken()->_pos
			).withSecondary(
				"try adding `{` here instead",
				pos
			));
		auto *block = new AST::StatementBlock();
		block->addStatement(parseStatement());
		return block;
	}
}

AST::Literal *Parser::convertToLiteral(AST::Expression *exp, string errorMsg)
{
	if (exp && exp->getExpressionType() != ET_LITERAL)
		throw ErrorReporter::report(errorMsg, ERR_GENERAL, exp->getPosition());
	return (AST::Literal*)exp;
}