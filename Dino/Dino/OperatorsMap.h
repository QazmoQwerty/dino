#pragma once

#include <vector>
#include <unordered_map>
#include "TypeEnums.h"
#include "Token.h"

using std::vector;
using std::string;
using std::pair;
using std::unordered_map;

#define SPECIAL 0
#define BINARY	0b0001
#define PREFIX	0b0010
#define POSTFIX	0b0100
//#define KEYWORD	0b1000
#define NONE -1

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
	static const unordered_map<string, Operator>& getWordOperators();

	static bool isWord(OperatorType type);
	static bool isUnary(OperatorType type);
	static bool isBinary(OperatorType type);
	static bool isKeyword(Operator op);
	static bool isAssignment(OperatorType type);

	static pair<const string, Operator> getOperatorByDefinition(OperatorType operatorType);
private:
	static unordered_map<string, Operator> _map;
	static unordered_map<string, Operator> _wordsMap;
};