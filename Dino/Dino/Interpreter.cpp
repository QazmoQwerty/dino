#include "Interpreter.h"

Value* Interpreter::interpret(AST::Node * node)
{
	if (node->isExpression())
	{
		auto expression = (AST::Expression*)node;
		switch (expression->getType())
		{
		case (ET_BINARY_OPERATION):
			return interpretBinaryOp((AST::BinaryOperation*)node);
		case (ET_UNARY_OPERATION):
			return interpretUnaryOp((AST::UnaryOperation*)node);
		case (ET_CONDITIONAL_EXPRESSION):
			break;
		case (ET_FUNCTION_CALL):
			interpretFuncCall((AST::FunctionCall*)node);
			break;
		case (ET_LITERAL):
			return interpretLiteral((AST::Literal*)node);
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
	case (OT_EQUAL):
		if (leftVal->getType() == "int") return new BoolValue(((IntValue*)leftVal)->getValue() == ((IntValue*)rightVal)->getValue());
		break;

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
	if (node->getFunctionId().name == "print")
		for (auto i : node->getParameters())
			std::cout << interpret(i)->toString() << std::endl;
	return nullptr;
}

Value * Interpreter::interpretLiteral(AST::Literal * node)
{
	switch (node->getLiteralType())
	{
	case (LT_BOOLEAN):
		return new BoolValue(((AST::Boolean*)node)->getValue());
	case (LT_INTEGER):
		return new IntValue(((AST::Boolean*)node)->getValue());
		break;
	case (LT_CHARACTER):
		break;
	case (LT_STRING):
		break;
	case (LT_FRACTION):
		break;
	case (LT_NULL):
		break;
	}
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
