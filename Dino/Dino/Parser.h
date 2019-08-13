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
	static void setup(vector<Token*>& tokens) { _tokens = tokens; }
	static AST::Node* Parse(int lastPrecedence);
private:
	static vector<Token*>& _tokens;
};