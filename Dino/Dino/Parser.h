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
	//Parser(vector<vector<Token*>*>& tokens) : _tokens(tokens) { _line = _index = _idCount = 0; }
	Parser(vector<Token*>& tokens) : _tokens(tokens) {_index = _idCount = 0; }
	//Token* getToken(unsigned int line, unsigned int index);
	Token* getToken(unsigned int index);
	//Token* peekToken() { return getToken(_line, _index); }
	Token* peekToken() { return getToken(_index); }
	Token* nextToken() { return getToken(_index++); }
	Token* nextToken(OperatorType expected);
	

	AST::Node* parse() { return parse(0); };
	AST::Node* parse(int lastPrecedence);
	int precedence(Token* token);
private:

	AST::Node* nud(Token* token);
	AST::Node* led(AST::Node * left, Token * token);

	//vector<vector<Token*>*>& _tokens;

	vector<Token*>& _tokens;

	unsigned int _idCount;
	//unsigned int _line;
	unsigned int _index;
};