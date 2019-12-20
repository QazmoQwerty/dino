#pragma once

#include <vector>
#include <unordered_map>
#include <string>
#include <iostream>
#include <stack>
#include "Token.h"
#include "OperatorsMap.h"
#include "AstNode.h"

class Parser
{
public:
	Parser(vector<Token*>& tokens) : _tokens(tokens) {_index = 0; }
	Token* getToken(unsigned int index);
	Token* peekToken() { return getToken(_index); }
	Token* nextToken() { return getToken(_index++); }
	
	AST::Node* parse(int lastPrecedence = 0);
	AST::StatementBlock* parseBlock(OperatorType expected = OT_EOF);

	void expectLineBreak();
	void expectOperator(OperatorType ot);
	unicode_string expectIdentifier();
	vector<unicode_string> expectIdentifierList();

	AST::Expression* convertToExpression(AST::Node* node);
	AST::Statement* convertToStatement(AST::Node* node);

	AST::Statement* parseStatement();
	AST::Expression* parseExpression(int precedence = 0);
	AST::Expression* parseOptionalExpression(int precedence = 0);
	AST::StatementBlock* parseInnerBlock();
	int precedence(Token* token, int category);
	int leftPrecedence(OperatorToken* token, int category);
private:

	bool isOperator(Token * token, OperatorType type) { return token->_type == TT_OPERATOR && ((OperatorToken*)token)->_operator._type == type; };
	bool eatOperator(OperatorType type) { if (isOperator(peekToken(), type)) { nextToken(); return true; } return false; }
	
	bool eatLineBreak() { if (peekToken()->_type == TT_LINE_BREAK) { nextToken(); return true; } return false; }

	void skipLineBreaks() { while (eatLineBreak()); }

	AST::Node* std(Token* token);
	AST::Node* nud(Token* token);
	AST::Node* led(AST::Node * left, Token * token);

	vector<Token*>& _tokens;
	unsigned int _index;
};