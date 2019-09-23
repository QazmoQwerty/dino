#include "Interpreter.h"

Value* Interpreter::interpret(AST::Node * node)
{
	if (node->isStatement())
	{
		auto statement = dynamic_cast<AST::Statement*>(node);
		switch (statement->getStatementType())
		{
			case(ST_ASSIGNMENT):
				return interpretAssignment(dynamic_cast<AST::Assignment*>(node));
			case(ST_VARIABLE_DECLARATION):
				interpretVariableDeclaration(dynamic_cast<AST::VariableDeclaration*>(node));
				break;
			case (ST_FUNCTION_CALL):
				return interpretFuncCall(dynamic_cast<AST::FunctionCall*>(node));
			case(ST_IF_THEN_ELSE):
				return interpretIfThenElse(dynamic_cast<AST::IfThenElse*>(node));
			case(ST_STATEMENT_BLOCK):
				for (auto i : (dynamic_cast<AST::StatementBlock*>(node))->getChildren())
				{
					Value* val = interpret(i);
					if (val != nullptr && val->isReturn())
						return val;
				}
				break;
			case(ST_WHILE_LOOP):
				interpretWhileLoop(dynamic_cast<AST::WhileLoop*>(node));
				break;
			case(ST_DO_WHILE_LOOP):
				interpretDoWhileLoop(dynamic_cast<AST::DoWhileLoop*>(node));
				break;
			case(ST_UNARY_OPERATION):
				return interpretUnaryOpStatement(dynamic_cast<AST::UnaryOperationStatement*>(node));
		}
	}
	else
	{
		//auto expression = (AST::Expression*)node;
		auto expression = dynamic_cast<AST::Expression*>(node);
		switch (expression->getExpressionType())
		{
			case (ET_BINARY_OPERATION):
				return interpretBinaryOp(dynamic_cast<AST::BinaryOperation*>(node));
			case (ET_UNARY_OPERATION):
				return interpretUnaryOp(dynamic_cast<AST::UnaryOperation*>(node));
			case (ET_CONDITIONAL_EXPRESSION):
				break;
			/*case (ET_FUNCTION_CALL):
				interpretFuncCall(dynamic_cast<AST::FunctionCall*>(node));
				break;*/
			case (ET_LITERAL):
				return interpretLiteral(dynamic_cast<AST::Literal*>(node));
				break;
			case (ET_VARIABLE):
				return interpretVariable(dynamic_cast<AST::Variable*>(node));
				break;
			default:
				break;
		}
	}

	return NULL;
}

Value * Interpreter::interpretAssignment(AST::Assignment * node)
{
	if (OperatorsMap::isAssignment(node->getOperator()._type))
	{
		if (node->getLeft()->getExpressionType() != ET_VARIABLE)
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
			else if (var->getType() == "func") ((FuncValue*)var)->setValue(((FuncValue*)rightVal)->getValue());
			return var;
		case (OT_ASSIGN_ADD):
			if (var->getType() == "int") ((IntValue*)var)->setValue(((IntValue*)var)->getValue() + ((IntValue*)rightVal)->getValue());
			else if (var->getType() == "frac") ((FracValue*)var)->setValue(((FracValue*)var)->getValue() + ((FracValue*)rightVal)->getValue());
			else if (var->getType() == "string") ((StringValue*)var)->setValue(((StringValue*)var)->getValue() + ((StringValue*)rightVal)->getValue());
			return var;
		case (OT_ASSIGN_SUBTRACT):
			if (var->getType() == "int") ((IntValue*)var)->setValue(((IntValue*)var)->getValue() - ((IntValue*)rightVal)->getValue());
			else if (var->getType() == "frac") ((FracValue*)var)->setValue(((FracValue*)var)->getValue() - ((FracValue*)rightVal)->getValue());
			return var;
		case (OT_ASSIGN_MULTIPLY):
			if (var->getType() == "int") ((IntValue*)var)->setValue(((IntValue*)var)->getValue() * ((IntValue*)rightVal)->getValue());
			else if (var->getType() == "frac") ((FracValue*)var)->setValue(((FracValue*)var)->getValue() * ((FracValue*)rightVal)->getValue());
			return var;
		case (OT_ASSIGN_DIVIDE):
			if (var->getType() == "int") ((IntValue*)var)->setValue(((IntValue*)var)->getValue() / ((IntValue*)rightVal)->getValue());
			else if (var->getType() == "frac") ((FracValue*)var)->setValue(((FracValue*)var)->getValue() / ((FracValue*)rightVal)->getValue());
			return var;
		case (OT_ASSIGN_MODULUS):
			if (var->getType() == "int") ((IntValue*)var)->setValue(((IntValue*)var)->getValue() % ((IntValue*)rightVal)->getValue());
			return var;
		}
	}
	return nullptr;
}

Value* Interpreter::interpretBinaryOp(AST::BinaryOperation * node)
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
	if (node->getFunctionId().name == "Print")
		for (auto i : node->getParameters())
			std::cout << interpret(i)->toString() << std::endl;
	else if (_variables.count(node->getFunctionId().name))
	{
		Value* var = _variables[node->getFunctionId().name];
		if (var->getType() != "func")
			throw "cannot invoke non-function value!";
		auto values = node->getParameters();

		auto params = ((FuncValue*)var)->getValue()->getParameters();

		if (params.size() != values.size())
			throw "Incorrect number of parameter inputs";
		for (unsigned int i = 0; i < params.size(); i++)
		{
			interpret(params[i]);
			auto assign = new AST::Assignment();
			string name = params[i]->getVarId().name;
			Value* val = interpret(values[i]);
			if (params[i]->getVarType().name != val->getType())
				throw "Incompatible function inputs";

			if (val->getType() == "int") ((IntValue*)_variables[name])->setValue(((IntValue*)val)->getValue());
			else if (val->getType() == "bool") ((BoolValue*)_variables[name])->setValue(((BoolValue*)val)->getValue());
			else if (val->getType() == "frac") ((FracValue*)_variables[name])->setValue(((FracValue*)val)->getValue());
			else if (val->getType() == "string") ((StringValue*)_variables[name])->setValue(((StringValue*)val)->getValue());
			else if (val->getType() == "char") ((CharValue*)_variables[name])->setValue(((CharValue*)val)->getValue());
			else if (val->getType() == "func") ((FuncValue*)_variables[name])->setValue(((FuncValue*)val)->getValue());
			else throw "custom types are not supported yet";
		}
		Value* ret = interpret(((FuncValue*)var)->getValue()->getContent());
		for (auto i : params)
		{
			delete _variables[i->getVarId().name];
			_variables.erase(i->getVarId().name);
		}
		return ret;
	}
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
	case (LT_FUNCTION):
		return new FuncValue((AST::Function*)node);
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

Value * Interpreter::interpretUnaryOpStatement(AST::UnaryOperationStatement * node)
{
	switch (node->getOperator()._type)
	{
	case (OT_RETURN):
		{
			Value* val = interpret(node->getExpression());
			val->setReturn();
			return val;
		}
	}
	return NULL;
}

void Interpreter::interpretVariableDeclaration(AST::VariableDeclaration * node)
{
	string type = node->getVarType().name;
	string name = node->getVarId().name;
	auto vec = node->getModifiers();
	if (std::find_if(std::begin(vec), std::end(vec), 
		[](AST::Identificator id) { return id.name == "func"; }) != std::end(vec))
	{
		if (type == "bool" || type == "int" || type == "frac" || type == "string" || type == "char" || type == "void")
			_variables[name] = new FuncValue(type);
		else throw "custom types are not implemented yet.";
	}
	else
	{
		if		(type == "bool")	_variables[name] = new BoolValue();
		else if (type == "int")		_variables[name] = new IntValue();
		else if (type == "frac")	_variables[name] = new FracValue();
		else if (type == "string")	_variables[name] = new StringValue();
		else if (type == "char")	_variables[name] = new CharValue();
		else throw "custom types are not implemented yet.";
	}
}

Value* Interpreter::interpretIfThenElse(AST::IfThenElse * node)
{
	Value* condition = interpret(node->getCondition());
	if (condition->getType() == "bool")
	{
		Value* val;
		if (((BoolValue*)condition)->getValue())
			val = interpret(node->getThenBranch());
		else val = interpret(node->getElseBranch());
		if (val != nullptr && val->isReturn()) return val;
	}
	return NULL;
}

Value* Interpreter::interpretWhileLoop(AST::WhileLoop * node)
{
	if (interpret(node->getCondition())->getType() == "bool")
		while (((BoolValue*)interpret(node->getCondition()))->getValue())
		{
			Value* val = interpret(node->getStatement());
			if (val != nullptr && val->isReturn()) return val;
		}
	return NULL;
}

Value* Interpreter::interpretDoWhileLoop(AST::DoWhileLoop * node)
{
	if (interpret(node->getCondition())->getType() == "bool")
		do {
			Value* val = interpret(node->getStatement());
			if (val != nullptr && val->isReturn()) return val;
		} while (((BoolValue*)interpret(node->getCondition()))->getValue());
	return NULL;
}
