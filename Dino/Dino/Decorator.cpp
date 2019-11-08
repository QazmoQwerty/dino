#include "Decorator.h"



Decorator::Decorator()
{
}


Decorator::~Decorator()
{
}

DST::Node *Decorator::decorate(AST::Node * node)
{
	return NULL;
}

DST::Expression * Decorator::convertToExpression(DST::Node * node)
{
	if (node == nullptr || node->isExpression())
		return dynamic_cast<DST::Expression*>(node);
	throw DinoException("expected an expression", EXT_GENERAL, node->getLine());
}

DST::Statement * Decorator::convertToStatement(DST::Node * node)
{
	if (node == nullptr || node->isStatement())
		return dynamic_cast<DST::Statement*>(node);
	throw DinoException("expected a statement", EXT_GENERAL, node->getLine());
}
