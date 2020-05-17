/*
    Utility functions for the decorator.
*/
#include "Decorator.h"

DST::Node *Decorator::decorate(AST::Node * node)
{
	if (node == nullptr) 	  return NULL;
	if (node->isExpression()) return decorate(dynamic_cast<AST::Expression*>(node));
	else 					  return decorate(dynamic_cast<AST::Statement*>(node));
}

DST::Type * Decorator::evalType(AST::Expression * node)
{
	auto ret = decorate(node);
	if (ret->getExpressionType() != ET_TYPE)
		throw ErrorReporter::report("expected a type", ErrorReporter::GENERAL_ERROR, ret->getPosition());
	return (DST::Type*)ret;
}

bool Decorator::isCondition(DST::Expression * node)
{
	return node && node->getType()->equals(DST::getBoolTy());
}

void Decorator::leaveBlock()
{
	for (auto i : _variables[currentScope()])
		_toDelete.push_back(i.second);
	_variables.pop_back();
}

void Decorator::clear()
{
	leaveBlock();
	for (auto i : _toDelete)
		if (i != nullptr)
		{
			try
			{
				delete i;
			}
			catch (...)
			{
				// continue;
			}
		}
}
