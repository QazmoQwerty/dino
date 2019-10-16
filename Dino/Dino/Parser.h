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
	Token* nextToken(OperatorType expected);
	

	AST::Node* parse() { return parse(0); };
	AST::Node* parse(int lastPrecedence);
	AST::StatementBlock* parseBlock() { return parseBlock(OT_EOF); };
	AST::StatementBlock* parseBlock(OperatorType expected);

	void expectLineBreak();
	void expectOperator(OperatorType ot);
	string expectIdentifier();
	vector<string> expectIdentifierList();

	AST::Statement* parseStatement();
	AST::Expression* parseExpression();
	AST::Statement* parseInnerBlock();
	//int precedence(Token* token);
	int precedence(Token* token, int category);
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