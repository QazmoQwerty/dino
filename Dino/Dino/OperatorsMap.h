#pragma once

#include <vector>
#include <unordered_map>
#include "TypeEnums.h"
#include "Token.h"

using std::vector;
using std::string;
using std::pair;
using std::unordered_map;

class OperatorsMap
{
public:
	/*
		Sets up internal maps.
		IMPORTANT: Function MUST be called once before using any other functions of this class.
	*/
	static void setup();

	/*
		Funtion returns map with an operator string as the key, and the corresponding OperatorType as the value.
	*/
	static const unordered_map<string, Operator>& getOperators();

	static pair<const string, Operator> getOperatorByDefinition(OperatorType operatorType);
private:
	static unordered_map<string, Operator> _map;
};