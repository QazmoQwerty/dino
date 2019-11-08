#pragma once

#include "DstNode.h"

class Decorator
{
public:
	Decorator();
	~Decorator();

	 static DST::Node *decorate(AST::Node *node);
	 static DST::Expression* convertToExpression(DST::Node* node);
	 static DST::Statement* convertToStatement(DST::Node* node);

};

