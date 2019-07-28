#pragma once

#include <vector>
#include <unordered_map>
#include "Token.h"

using std::vector;
using std::string;
using std::unordered_map;

class OperatorsMap
{
public:
	static void setup();
	static unordered_map<string, OperatorType>& getOperators();
private:
	static unordered_map<string, OperatorType> _map;
};