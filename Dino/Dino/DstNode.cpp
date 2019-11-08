#include "DstNode.h"

DST::BinaryOperation::BinaryOperation(AST::BinaryOperation * base) : _base(base)
{
	_left = Decorator::convertToExpression(Decorator::decorate(_base->getLeft()));
	_right = Decorator::convertToExpression(Decorator::decorate(_base->getRight()));
}

DST::ConditionalExpression::ConditionalExpression(AST::ConditionalExpression * base) : _base(base)
{
	_condition = Decorator::convertToExpression(Decorator::decorate(_base->getCondition()));
	_thenBranch = Decorator::convertToExpression(Decorator::decorate(_base->getThenBranch()));
	_elseBranch = Decorator::convertToExpression(Decorator::decorate(_base->getElseBranch()));
}
