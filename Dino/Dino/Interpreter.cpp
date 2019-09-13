#include "Interpreter.h"

Value* Interpreter::interpret(AST::Node * node)
{
	if (node->isExpression())
	{
		auto expression = (AST::Expression*)node;
		switch (expression->getType())
		{
		case (ET_BINARY_OPERATION):
			break;
		case (ET_UNARY_OPERATION):
			break;
		case (ET_CONDITIONAL_EXPRESSION):
			break;
		case (ET_FUNCTION_CALL):
			break;
		case (ET_LITERAL):
			break;
		case (ET_VARIABLE):
			break;
		default:
			break;
		}
	}
	else
	{
		auto statement = (AST::Statement*)node;
		switch (statement->getType())
		{
		case(ST_ASSIGNMENT):
			break;
		case(ST_IF_THEN_ELSE):
			break;
		case(ST_STATEMENT_BLOCK):
			break;
		case(ST_VARIABLE_DECLARATION):
			break;
		case(ST_WHILE_LOOP):
			break;
		default:
			break;
		}
	}

	return 0;
}
