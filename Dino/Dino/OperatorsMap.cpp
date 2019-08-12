#include "OperatorsMap.h"

unordered_map<string, OperatorType> OperatorsMap::_map;

const unordered_map<string, OperatorType>& OperatorsMap::getOperators() { return _map; }

/*
	Gets an OperatorType and searches _map for the corresponding operator string.
	NOTE: Unused function, might get deleted in the future.
*/
pair<const string, OperatorType> OperatorsMap::getOperatorByDefinition(OperatorType operatorType)
{
	for (auto t : OperatorsMap::getOperators())
		if (t.second == operatorType)
			return t;
	return pair<const string, OperatorType>("", OT_UNKNOWN);
}

/*
	Sets up values in _map.
	IMPORTANT: Function MUST be called once before using any other functions of this class.
*/
void OperatorsMap::setup()
{
	_map = unordered_map<string, OperatorType>();

	// Arithmetic Operators:
	_map["+"] = OT_ADD;
	_map["-"] = OT_SUBTRACT;
	_map["/"] = OT_DIVIDE;
	_map["*"] = OT_MULTIPLY;
	_map["%"] = OT_MODULUS;
	_map["++"] = OT_INCREMENT;
	_map["--"] = OT_DECREMENT;

	// Relational Operators:
	_map["=="] = OT_EQUAL;
	_map["!+"] = OT_NOT_EQUAL;
	_map[">"] = OT_GREATER;
	_map["<"] = OT_SMALLER;
	_map[">="] = OT_GREATER_EQUAL;
	_map["<="] = OT_SMALLER_EQUAL;

	// Bitwise Operators:
	_map["&"] = OT_BITWISE_AND;
	_map["|"] = OT_BITWISE_OR;
	_map["^"] = OT_BITWISE_XOR;
	_map["~"] = OT_BITWISE_NOT;
	_map["<<"] = OT_BITWISE_SHIFT_LEFT;
	_map[">>"] = OT_BITWISE_SHIFT_RIGHT;

	// Logical Operators:
	_map["&&"] = OT_LOGICAL_AND;
	_map["||"] = OT_LOGICAL_OR;
	_map["!"] = OT_LOGICAL_NOT;

	// Assignment Operators:
	_map["="] = OT_ASSIGN_EQUAL;
	_map["+="] = OT_ASSIGN_ADD;
	_map["-="] = OT_ASSIGN_SUBTRACT;
	_map["*="] = OT_ASSIGN_MULTIPLY;
	_map["/="] = OT_ASSIGN_DIVIDE;
	_map["%="] = OT_ASSIGN_MODULUS;
	_map["<<="] = OT_ASSIGN_SHIFT_LEFT;
	_map[">>="] = OT_ASSIGN_SHIFT_RIGHT;
	_map["&="] = OT_ASSIGN_BITWISE_AND;
	_map["|="] = OT_ASSIGN_BITWISE_OR;
	_map["^="] = OT_ASSIGN_BITWISE_XOR;

	// Brackets / Parenthesis:
	_map["("] = OT_PARENTHESIS_OPEN;
	_map[")"] = OT_PARENTHESIS_CLOSE;
	_map["["] = OT_SQUARE_BRACKETS_OPEN;
	_map["]"] = OT_SQUARE_BRACKETS_CLOSE;
	_map["{"] = OT_CURLY_BRACES_OPEN;
	_map["}"] = OT_CURLY_BRACES_CLOSE;

	// Comments:
	_map["//"] = OT_SINGLE_LINE_COMMENT;
	_map["/*"] = OT_MULTI_LINE_COMMENT_OPEN;
	_map["*/"] = OT_MULTI_LINE_COMMENT_CLOSE;

	// Misc:
	_map["'"] = OT_SINGLE_QUOTE;
	_map["\""] = OT_DOUBLE_QUOTE;
	_map["."] = OT_PERIOD;
	_map[","] = OT_COMMA;
	_map[":"] = OT_COLON;
	_map["\\"] = OT_ESCAPE;
}
