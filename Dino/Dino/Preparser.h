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
	The Preparser gets a vector of tokens, and cleans it up to make the Parser's job easier.
	The final result is a two dimensional vector of tokens, in which each individual vector represents a line of code.
*/

class Preparser
{
public:
	static vector<vector<Token*>>& Preparse(vector<Token*> tokens);
private:

};
