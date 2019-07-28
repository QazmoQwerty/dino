#include "OperatorsMap.h"

void OperatorsMap::setup()
{
	_map = unordered_map<string, OperatorType>();
	_map["//"] = OT_UNKNOWN;
}

unordered_map<std::string, OperatorType>& OperatorsMap::getOperators()
{
	return _map;
}
