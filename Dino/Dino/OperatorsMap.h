#pragma once

#include <vector>
#include <unordered_map>
#include "Token.h"

using std::vector;
using std::string;
using std::pair;
using std::unordered_map;

#define LEFT_TO_RIGHT true
#define RIGHT_TO_LEFT false

typedef struct Operator
{
	OperatorType _type;
	string _str;
	bool _associativity;
	unsigned int _precedence; // Lower value means HIGHER precedence - first precedense is 1.
} Operator;

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
	static const unordered_map<string, OperatorType>& getOperators();

	static pair<const string, OperatorType> getOperatorByDefinition(OperatorType operatorType);
private:
	static unordered_map<string, OperatorType> _map;
};