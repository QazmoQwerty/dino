#pragma once

#include <vector>
#include <unordered_map>
#include <string>
#include <iostream>
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
	Parser(vector<vector<Token*>*>& tokens) : _tokens(tokens) { _line = _index = 0; }
	Token* getToken(int line, int index) { return (*_tokens[line])[index]; }
	AST::Node* Parse(unsigned int lastPrecedence);
	unsigned int calcPrecedence(Token *token);
private:

	AST::Node* nud(Token* token);
	AST::Node* led(Token* left, Token* current);

	vector<vector<Token*>*>& _tokens;
	int _line;
	int _index;
};