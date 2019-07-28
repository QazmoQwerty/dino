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
	_map["+"] = OT_UNKNOWN;
	_map["-"] = OT_UNKNOWN;
	_map["/"] = OT_UNKNOWN;
	_map["*"] = OT_UNKNOWN;
	_map["%"] = OT_UNKNOWN;
	_map["("] = OT_UNKNOWN;
	_map[")"] = OT_UNKNOWN;
	_map["("] = OT_UNKNOWN;
	_map[")"] = OT_UNKNOWN;
	_map["=="] = OT_UNKNOWN;
}
