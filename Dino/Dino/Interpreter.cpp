#include "Interpreter.h"

Value* Interpreter::interpret(AST::Node * node)
{
	if (node->isExpression())
	{
		auto expression = (AST::Expression*)node;
		switch (expression->getType())
		{
		case (ET_BINARY_OPERATION):
			return interpretBinaryOp((AST::BinaryOperation*)expression);
		case (ET_UNARY_OPERATION):
			return interpretUnaryOp((AST::UnaryOperation*)expression);
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
		/*case(ST_ASSIGNMENT):
			break;*/
		case(ST_IF_THEN_ELSE):
			interpretIfThenElse((AST::IfThenElse*)node);
			break;
		case(ST_STATEMENT_BLOCK):
			for (auto i : ((AST::StatementBlock*)node)->getChildren())
				interpret(i);
			break;
		case(ST_VARIABLE_DECLARATION):
			
			break;
		case(ST_WHILE_LOOP):
			interpretWhileLoop((AST::WhileLoop*)node);
			break;
		}
	}

	return NULL;
}

Value* Interpreter::interpretBinaryOp(AST::BinaryOperation * node)
{
	auto binaryOp = ((AST::BinaryOperation*)node);
	// TODO - assignment operators

	Value* leftVal = interpret(binaryOp->getLeft());
	Value* rightVal = interpret(binaryOp->getRight());
	if (leftVal->getType() != rightVal->getType())
		throw "different types invalid";
	switch (binaryOp->getOperator()._type)
	{
	case(OT_ADD):
		if (leftVal->getType() == "int") return new IntValue(((IntValue*)leftVal)->getValue() + ((IntValue*)rightVal)->getValue());
		break;
	case(OT_SUBTRACT):
		if (leftVal->getType() == "int") return new IntValue(((IntValue*)leftVal)->getValue() - ((IntValue*)rightVal)->getValue());
		break;
	case(OT_MULTIPLY):
		if (leftVal->getType() == "int") return new IntValue(((IntValue*)leftVal)->getValue() * ((IntValue*)rightVal)->getValue());
		break;
	case(OT_DIVIDE):
		if (leftVal->getType() == "int") return new IntValue(((IntValue*)leftVal)->getValue() / ((IntValue*)rightVal)->getValue());
		break;
	case(OT_MODULUS):
		if (leftVal->getType() == "int") return new IntValue(((IntValue*)leftVal)->getValue() % ((IntValue*)rightVal)->getValue());
		break;
	default:
		break;
	}
	return NULL;
}

Value * Interpreter::interpretUnaryOp(AST::UnaryOperation * node)
{
	return nullptr;
}

Value * Interpreter::interpretFuncCall(AST::FunctionCall * node)
{
	
	return nullptr;
}

void Interpreter::interpretIfThenElse(AST::IfThenElse * node)
{
	Value* condition = interpret(node->getCondition());
	if (condition->getType() == "bool")
	{
		if (((BoolValue*)condition)->getValue())
			interpret(node->getThenBranch());
		else interpret(node->getElseBranch());
	}
}

void Interpreter::interpretWhileLoop(AST::WhileLoop * node)
{
	if (interpret(node->getCondition())->getType() == "bool")
		while (((BoolValue*)interpret(node->getCondition()))->getValue())
			interpret(node->getStatement());
}
