#pragma once

#include <vector>
#include <unordered_map>
#include "../TypeEnums.h"
#include "../../Lexer/Token.h"

using std::vector;
using std::string;
using std::pair;
using std::unordered_map;

#define SPECIAL 0
#define BINARY	1	// 0b0001
#define PREFIX	2	// 0b0010
#define POSTFIX	4	// 0b0100
#define NONE   -1
#define NON_ASSCOCIATIVE 0

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

	/* Is the operator a word operator? */
	static bool isWord(OperatorType type);

	/* Is the operator a unary operator? */
	static bool isUnary(OperatorType type);

	/* Is the operator a binary operator? */
	static bool isBinary(OperatorType type);

	/* Is the operator a keyword? */
	static bool isKeyword(Operator op);

	/* Is the operator an assignment operator? */
	static bool isAssignment(OperatorType type);

	/*
		Get output type of an operator. 
		Options are:
			RT_BOOLEAN 	- boolean type
			RT_LEFT 	- left operand's type
			RT_ARRAY 	- array member's type
			RT_VOID 	- no type (operator is does not create expressions)
	*/
	static ReturnType getReturnType(OperatorType type);

	/* Get a (string, operator) pair corresponding to an OperatorType */
	static pair<const unicode_string, Operator> getOperatorByDefinition(OperatorType operatorType);
private:
	static unordered_map<unicode_string, Operator, UnicodeHasherFunction> _map;
	static unordered_map<unicode_string, Operator, UnicodeHasherFunction> _wordsMap;
};