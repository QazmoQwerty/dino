#include "OperatorsMap.h"

unordered_map<string, OperatorType> OperatorsMap::_map;

unordered_map<string, OperatorType>& OperatorsMap::getOperators()
{
	return _map;
}

void OperatorsMap::setup()
{
	_map = unordered_map<string, OperatorType>();
	// temporary for testing
	_map["+"] = OT_ADD;
	_map["-"] = OT_SUBTRACT;
	_map["/"] = OT_DIVIDE;
	_map["*"] = OT_MULTIPLY;
	_map["%"] = OT_MODULUS;
	_map["("] = OT_PARENTHESIS_OPEN;
	_map[")"] = OT_PARENTHESIS_CLOSE;
	_map["=="] = OT_EQUAL;
	_map["."] = OT_PERIOD;
}
