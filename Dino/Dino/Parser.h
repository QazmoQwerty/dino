#pragma once

#include <vector>
#include <unordered_map>
#include <string>
#include <iostream>
#include <stack>
#include "Token.h"
#include "OperatorsMap.h"
#include "AstNode.h"

// TODO - do we need all of these?
using std::vector;	
using std::string;
using std::pair;
using std::unordered_map;

class Parser
{
public:
	Parser(vector<vector<Token*>*>& tokens) : _tokens(tokens) { _line = _index = _idCount = 0; }
	Token* getToken(unsigned int line, unsigned int index);
	Token* peekToken() { return getToken(_line, _index); }
	Token* nextToken();
	Token* nextToken(OperatorType expected);
	

	AST::Node* parse() { return parse(0); };
	//AST::Node* parse(OperatorType expected) { return parse(0, expected); };
	AST::Node* parse(int lastPrecedence);
	//AST::Node* parse(int lastPrecedence, OperatorType expected);
	int precedence(Token* token);
private:

	AST::Node* nud(Token* token);
	//AST::Node* nud(Token* token, OperatorType expected);
	AST::Node* led(AST::Node * left, Token * token);
	//AST::Node* led(AST::Node * left, Token * token, OperatorType expected);

	vector<vector<Token*>*>& _tokens;

	unsigned int _idCount;
	unsigned int _line;
	unsigned int _index;
};