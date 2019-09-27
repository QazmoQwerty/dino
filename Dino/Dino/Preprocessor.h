#pragma once

#include <vector>
#include <unordered_map>
#include <string>
#include <iostream>
#include "Token.h"
#include "OperatorsMap.h"

using std::vector;
using std::string;
using std::pair;
using std::unordered_map;

/*
	The Preprocessor gets a vector of tokens, and cleans it up to make the Parser's job easier.
	The final result is a two dimensional vector of tokens, in which each individual vector represents a line of code.
	Some examles of things the Preprocessor does:
		1) Discards any whitespace/comment tokens.
		2) Deals with special operators which the Lexer cannot recognise (such as word operators - if, while, else).
*/

class Preprocessor 
{
public:
	//static vector<vector<Token*>*>& Preparse(vector<Token*> tokens);
	static vector<Token*>& preprocess(vector<Token*> tokens);
};
