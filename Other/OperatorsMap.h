#pragma once

#include <vector>
#include <unordered_map>
#include "TypeEnums.h"
#include "../Lexer/Token.h"

using std::vector;
using std::string;
using std::pair;
using std::unordered_map;

#define SPECIAL 0
#define BINARY	0b0001
#define PREFIX	0b0010
#define POSTFIX	0b0100
#define NONE -1

class OperatorsMap
{
public:
	/*
		Sets up internal maps.
		IMPORTANT: Function MUST be called once before using any other functions of this class.
	*/
	static void setup();

	/* Funtion returns map with an operator string as the key, and the corresponding OperatorType as the value. */
	static const unordered_map<unicode_string, Operator, UnicodeHasherFunction>& getOperators();

	/* Funtion returns map with an operator string as the key, and the corresponding OperatorType as the value. */
	static const unordered_map<unicode_string, Operator, UnicodeHasherFunction>& getWordOperators();

	static bool isWord(OperatorType type);
	static bool isUnary(OperatorType type);
	static bool isBinary(OperatorType type);
	static bool isKeyword(Operator op);
	static bool isAssignment(OperatorType type);

	static ReturnType getReturnType(OperatorType type);

	static pair<const unicode_string, Operator> getOperatorByDefinition(OperatorType operatorType);
private:
	static unordered_map<unicode_string, Operator, UnicodeHasherFunction> _map;
	static unordered_map<unicode_string, Operator, UnicodeHasherFunction> _wordsMap;
};