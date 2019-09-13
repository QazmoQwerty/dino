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
			return interpretVariable((AST::Variable*)node);
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
			interpretVariableDeclaration((AST::VariableDeclaration*)node);
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
	// TODO - assignment operators
	if (OperatorsMap::isAssignment(node->getOperator()._type))
	{
		if (node->getLeft()->getType() != ET_VARIABLE)
			throw "cannot assign to anything but a variable!";

		Value* rightVal = interpret(node->getRight());
		Value* var = interpretVariable((AST::Variable*)node->getLeft());
		if (var->getType() != rightVal->getType())
			throw "different types invalid";
		switch (node->getOperator()._type)
		{
		case (OT_ASSIGN_EQUAL):
			if (var->getType() == "int") ((IntValue*)var)->setValue(((IntValue*)rightVal)->getValue());
			else if (var->getType() == "bool") ((BoolValue*)var)->setValue(((BoolValue*)rightVal)->getValue());
			else if (var->getType() == "frac") ((FracValue*)var)->setValue(((FracValue*)rightVal)->getValue());
			else if (var->getType() == "string") ((StringValue*)var)->setValue(((StringValue*)rightVal)->getValue());
			else if (var->getType() == "char") ((CharValue*)var)->setValue(((CharValue*)rightVal)->getValue());
			return var;
			break;
		}
	}
	else
	{
		Value* leftVal = interpret(node->getLeft());
		Value* rightVal = interpret(node->getRight());
		if (leftVal->getType() != rightVal->getType())
			throw "different types invalid";
		switch (node->getOperator()._type)
		{
		case (OT_EQUAL):
			if (leftVal->getType() == "int") return new BoolValue(((IntValue*)leftVal)->getValue() == ((IntValue*)rightVal)->getValue());
			break;
		case (OT_SMALLER):
			if (leftVal->getType() == "int") return new BoolValue(((IntValue*)leftVal)->getValue() < ((IntValue*)rightVal)->getValue());
			break;
		case (OT_LOGICAL_AND):
			if (leftVal->getType() == "bool") return new BoolValue(((BoolValue*)leftVal)->getValue() && ((BoolValue*)rightVal)->getValue());
			break;

		case(OT_ADD):
			if (leftVal->getType() == "int") return new IntValue(((IntValue*)leftVal)->getValue() + ((IntValue*)rightVal)->getValue());
			if (leftVal->getType() == "frac") return new FracValue(((FracValue*)leftVal)->getValue() + ((FracValue*)rightVal)->getValue());
			if (leftVal->getType() == "string") return new StringValue(((StringValue*)leftVal)->getValue() + ((StringValue*)rightVal)->getValue());
			break;
		case(OT_SUBTRACT):
			if (leftVal->getType() == "int") return new IntValue(((IntValue*)leftVal)->getValue() - ((IntValue*)rightVal)->getValue());
			if (leftVal->getType() == "frac") return new FracValue(((FracValue*)leftVal)->getValue() - ((FracValue*)rightVal)->getValue());
			break;
		case(OT_MULTIPLY):
			if (leftVal->getType() == "int") return new IntValue(((IntValue*)leftVal)->getValue() * ((IntValue*)rightVal)->getValue());
			if (leftVal->getType() == "frac") return new FracValue(((FracValue*)leftVal)->getValue() * ((FracValue*)rightVal)->getValue());
			break;
		case(OT_DIVIDE):
			if (leftVal->getType() == "int") return new IntValue(((IntValue*)leftVal)->getValue() / ((IntValue*)rightVal)->getValue());
			if (leftVal->getType() == "frac") return new FracValue(((FracValue*)leftVal)->getValue() / ((FracValue*)rightVal)->getValue());
			break;
		case(OT_MODULUS):
			if (leftVal->getType() == "int") return new IntValue(((IntValue*)leftVal)->getValue() % ((IntValue*)rightVal)->getValue());
			break;
		default:
			break;
		}
	}
	return NULL;
}

Value * Interpreter::interpretUnaryOp(AST::UnaryOperation * node)
{
	Value* val = interpret(node->getExpression());
	switch (node->getOperator()._type)
	{
	case (OT_ADD):
		return val;
	case (OT_SUBTRACT):
		if (val->getType() == "int") ((IntValue*)val)->setValue(-((IntValue*)val)->getValue());
		return val;
	case (OT_INCREMENT):
		if (val->getType() == "int") ((IntValue*)val)->setValue(((IntValue*)val)->getValue() + 1);
		return val;
	case (OT_DECREMENT):
		if (val->getType() == "int") ((IntValue*)val)->setValue(((IntValue*)val)->getValue() - 1);
		return val;
	case (OT_BITWISE_NOT):
		if (val->getType() == "int") ((IntValue*)val)->setValue(~((IntValue*)val)->getValue());
		return val;
	case (OT_LOGICAL_NOT):
		if (val->getType() == "bool") ((BoolValue*)val)->setValue(!((BoolValue*)val)->getValue());
		return val;
	}
	return NULL;
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
		return new CharValue(((AST::Character*)node)->getValue());
		break;
	case (LT_STRING):
		return new StringValue(((AST::String*)node)->getValue());
		break;
	case (LT_FRACTION):
		return new FracValue(((AST::Fraction*)node)->getValue());
		break;
	case (LT_NULL):
		break;
	}
	return nullptr;
}

Value * Interpreter::interpretVariable(AST::Variable * node)
{
	if (_variables.count(node->getVarId().name) == 0)
		throw "variable does not exist";
	return _variables[node->getVarId().name];
}

void Interpreter::interpretVariableDeclaration(AST::VariableDeclaration * node)
{
	string type = node->getVarType().name;
	string name = node->getVarId().name;
	if		(type == "bool")	_variables[name] = new BoolValue();
	else if (type == "int")		_variables[name] = new IntValue();
	else if (type == "frac")	_variables[name] = new FracValue();
	else if (type == "string")	_variables[name] = new StringValue();
	else if (type == "char")	_variables[name] = new CharValue();
	else throw "custom types are not implemented yet.";
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
