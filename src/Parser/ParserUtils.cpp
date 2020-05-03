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
