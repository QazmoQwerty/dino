#include "Interpreter.h"

#define deleteIfIsTemp(n) { if (n != nullptr && n->isTemp()) delete n; }

void Interpreter::leaveBlock()
{
	for (auto i : _variables[currentScope()])
		delete i.second;
	_variables.pop_back();
}

Value* Interpreter::interpret(AST::Node * node)
{
	if (node == nullptr)
		return NULL;
	if (node->isStatement())
	{
		auto statement = dynamic_cast<AST::Statement*>(node);
		switch (statement->getStatementType())
		{
			case(ST_ASSIGNMENT):
				return interpretAssignment(dynamic_cast<AST::Assignment*>(node));
			case(ST_VARIABLE_DECLARATION):
				return interpretVariableDeclaration(dynamic_cast<AST::VariableDeclaration*>(node));
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
				return interpretWhileLoop(dynamic_cast<AST::WhileLoop*>(node));
				break;
			case(ST_INCREMENT):
				return interpretIncrement(dynamic_cast<AST::Increment*>(node));
				break;
			case(ST_DO_WHILE_LOOP):
				return interpretDoWhileLoop(dynamic_cast<AST::DoWhileLoop*>(node));
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
	// TODO - should return copy of lvalue, not a reference to it
	Value* ret = nullptr;
	if (OperatorsMap::isAssignment(node->getOperator()._type))
	{
		Value* rvalue = interpret(node->getRight());
		Value* lvalue = interpret(node->getLeft());
		
		string type = lvalue->getType();

		if (type != rvalue->getType())
			throw "different types invalid";

		
		switch (node->getOperator()._type)
		{
		case (OT_ASSIGN_EQUAL):
			if		(type == "int") ((IntValue*)lvalue)->setValue(((IntValue*)rvalue)->getValue());
			else if (type == "bool") ((BoolValue*)lvalue)->setValue(((BoolValue*)rvalue)->getValue());
			else if (type == "frac") ((FracValue*)lvalue)->setValue(((FracValue*)rvalue)->getValue());
			else if (type == "string") ((StringValue*)lvalue)->setValue(((StringValue*)rvalue)->getValue());
			else if (type == "char") ((CharValue*)lvalue)->setValue(((CharValue*)rvalue)->getValue());
			else if (type == "func") ((FuncValue*)lvalue)->setValue(((FuncValue*)rvalue)->getValue());
			break;
		case (OT_ASSIGN_ADD):
			if		(type == "int") ((IntValue*)lvalue)->setValue(((IntValue*)lvalue)->getValue() + ((IntValue*)rvalue)->getValue());
			else if (type == "frac") ((FracValue*)lvalue)->setValue(((FracValue*)lvalue)->getValue() + ((FracValue*)rvalue)->getValue());
			else if (type == "string") ((StringValue*)lvalue)->setValue(((StringValue*)lvalue)->getValue() + ((StringValue*)rvalue)->getValue());
			break;
		case (OT_ASSIGN_SUBTRACT):
			if		(type == "int") ((IntValue*)lvalue)->setValue(((IntValue*)lvalue)->getValue() - ((IntValue*)rvalue)->getValue());
			else if (type == "frac") ((FracValue*)lvalue)->setValue(((FracValue*)lvalue)->getValue() - ((FracValue*)rvalue)->getValue());
			break;
		case (OT_ASSIGN_MULTIPLY):
			if		(type == "int") ((IntValue*)lvalue)->setValue(((IntValue*)lvalue)->getValue() * ((IntValue*)rvalue)->getValue());
			else if (type == "frac") ((FracValue*)lvalue)->setValue(((FracValue*)lvalue)->getValue() * ((FracValue*)rvalue)->getValue());
			break;
		case (OT_ASSIGN_DIVIDE):
			if		(type == "int") ((IntValue*)lvalue)->setValue(((IntValue*)lvalue)->getValue() / ((IntValue*)rvalue)->getValue());
			else if (type == "frac") ((FracValue*)lvalue)->setValue(((FracValue*)lvalue)->getValue() / ((FracValue*)rvalue)->getValue());
			break;
		case (OT_ASSIGN_MODULUS):
			if		(type == "int") ((IntValue*)lvalue)->setValue(((IntValue*)lvalue)->getValue() % ((IntValue*)rvalue)->getValue());
			break;
		}
		deleteIfIsTemp(rvalue);
		return lvalue;
	}
	return ret;
}

Value* Interpreter::interpretBinaryOp(AST::BinaryOperation * node)
{	
	Value* leftVal = interpret(node->getLeft());
	Value* rightVal = interpret(node->getRight());
	Value* ret = nullptr;
	if (leftVal->getType() != rightVal->getType())
		throw "different types invalid";
	switch (node->getOperator()._type)
	{
		case (OT_EQUAL):
			if (leftVal->getType() == "int") ret = new BoolValue(((IntValue*)leftVal)->getValue() == ((IntValue*)rightVal)->getValue());
			break;
		case (OT_SMALLER):
			if (leftVal->getType() == "int") ret = new BoolValue(((IntValue*)leftVal)->getValue() < ((IntValue*)rightVal)->getValue());
			break;
		case (OT_SMALLER_EQUAL):
			if (leftVal->getType() == "int") ret = new BoolValue(((IntValue*)leftVal)->getValue() <= ((IntValue*)rightVal)->getValue());
			break;
		case (OT_GREATER):
			if (leftVal->getType() == "int") ret = new BoolValue(((IntValue*)leftVal)->getValue() > ((IntValue*)rightVal)->getValue());
			break;
		case (OT_GREATER_EQUAL):
			if (leftVal->getType() == "int") ret = new BoolValue(((IntValue*)leftVal)->getValue() >= ((IntValue*)rightVal)->getValue());
			break;
		case (OT_LOGICAL_AND):
			if (leftVal->getType() == "bool") ret = new BoolValue(((BoolValue*)leftVal)->getValue() && ((BoolValue*)rightVal)->getValue());
			break;
		case(OT_ADD):
			if (leftVal->getType() == "int") ret = new IntValue(((IntValue*)leftVal)->getValue() + ((IntValue*)rightVal)->getValue());
			if (leftVal->getType() == "frac") ret = new FracValue(((FracValue*)leftVal)->getValue() + ((FracValue*)rightVal)->getValue());
			if (leftVal->getType() == "string") ret = new StringValue(((StringValue*)leftVal)->getValue() + ((StringValue*)rightVal)->getValue());
			break;
		case(OT_SUBTRACT):
			if (leftVal->getType() == "int") ret = new IntValue(((IntValue*)leftVal)->getValue() - ((IntValue*)rightVal)->getValue());
			if (leftVal->getType() == "frac") ret = new FracValue(((FracValue*)leftVal)->getValue() - ((FracValue*)rightVal)->getValue());
			break;
		case(OT_MULTIPLY):
			if (leftVal->getType() == "int") ret = new IntValue(((IntValue*)leftVal)->getValue() * ((IntValue*)rightVal)->getValue());
			if (leftVal->getType() == "frac") ret = new FracValue(((FracValue*)leftVal)->getValue() * ((FracValue*)rightVal)->getValue());
			break;
		case(OT_DIVIDE):
			if (leftVal->getType() == "int") ret = new IntValue(((IntValue*)leftVal)->getValue() / ((IntValue*)rightVal)->getValue());
			if (leftVal->getType() == "frac") ret = new FracValue(((FracValue*)leftVal)->getValue() / ((FracValue*)rightVal)->getValue());
			break;
		case(OT_MODULUS):
			if (leftVal->getType() == "int") ret = new IntValue(((IntValue*)leftVal)->getValue() % ((IntValue*)rightVal)->getValue());
			break;
		default: throw "unsupported binary operator";
	}
	deleteIfIsTemp(leftVal);
	deleteIfIsTemp(rightVal);
	return ret;
}

Value * Interpreter::interpretUnaryOp(AST::UnaryOperation * node)
{
	Value* val = interpret(node->getExpression());
	Value* ret = nullptr;
	switch (node->getOperator()._type)
	{ 
	case (OT_ADD):
		break;
	case (OT_SUBTRACT):
		if (val->getType() == "int") ret = new IntValue(-((IntValue*)val)->getValue());
		break;
	case (OT_INCREMENT):
		if (val->getType() == "int") ret = new IntValue(((IntValue*)val)->getValue() + 1);
		break;
	case (OT_DECREMENT):
		if (val->getType() == "int") ret = new IntValue(((IntValue*)val)->getValue() - 1);
		break;
	case (OT_BITWISE_NOT):
		if (val->getType() == "int") ret = new IntValue(~((IntValue*)val)->getValue());
		break;
	case (OT_LOGICAL_NOT):
		if (val->getType() == "bool") ret = new BoolValue(!((BoolValue*)val)->getValue());
		break;
	default: throw "unsupported unary operator";
	}
	deleteIfIsTemp(val);
	return ret;
}

Value * Interpreter::interpretIncrement(AST::Increment * node)
{
	Value* val = interpret(node->getExpression());
	switch (node->getOperator()._type)
	{
		case (OT_INCREMENT):
			if (val->getType() == "int") ((IntValue*)val)->setValue(((IntValue*)val)->getValue() + 1);
			return val;
		case (OT_DECREMENT):
			if (val->getType() == "int") ((IntValue*)val)->setValue(((IntValue*)val)->getValue() - 1);
			return val;
	}
	return nullptr;
}

Value * Interpreter::interpretFuncCall(AST::FunctionCall * node)
{
	if (node->getFunctionId()->getExpressionType() == ET_VARIABLE
		&& ((AST::Variable*)node->getFunctionId())->getVarId().name == "Print") 
	{
		for (auto i : node->getParameters()) 
		{
			Value* val = interpret(i);
			std::cout << val->toString() << std::endl;
		}
		return NULL;
	}
	enterBlock();
	Value* val = interpret(node->getFunctionId());
	if (val->getType() != "func")
		throw "cannot invoke non-function!";
	auto func = ((FuncValue*)val);
	auto values = node->getParameters();
	auto params = func->getValue()->getParameters();

	if (params.size() != values.size())
		throw "Incorrect number of parameter inputs";

	for (unsigned int i = 0; i < params.size(); i++)
	{
		Value* rvalue = interpret(values[i]);
		Value* lvalue = interpret(params[i]);

		string name = params[i]->getVarId().name;
		if (lvalue->getType() != rvalue->getType())
			throw "Incompatible function inputs";
		
		string type = lvalue->getType();
		if (type == "int")		((IntValue*)lvalue)->setValue(((IntValue*)rvalue)->getValue());
		else if (type == "bool")	((BoolValue*)lvalue)->setValue(((BoolValue*)rvalue)->getValue());
		else if (type == "frac")	((FracValue*)lvalue)->setValue(((FracValue*)rvalue)->getValue());
		else if (type == "string") ((StringValue*)lvalue)->setValue(((StringValue*)rvalue)->getValue());
		else if (type == "char")	((CharValue*)lvalue)->setValue(((CharValue*)rvalue)->getValue());
		else if (type == "func")	((FuncValue*)lvalue)->setValue(((FuncValue*)rvalue)->getValue());
		else throw "custom types are not supported yet";
	}
	Value* ret = interpret(func->getValue()->getContent());
	leaveBlock();
	return ret;
}

Value * Interpreter::interpretLiteral(AST::Literal * node)
{
	switch (node->getLiteralType())
	{
	case (LT_BOOLEAN):	 return new BoolValue(((AST::Boolean*)node)->getValue());
	case (LT_INTEGER):	 return new IntValue(((AST::Boolean*)node)->getValue());
	case (LT_CHARACTER): return new CharValue(((AST::Character*)node)->getValue());
	case (LT_STRING):	 return new StringValue(((AST::String*)node)->getValue());
	case (LT_FRACTION):  return new FracValue(((AST::Fraction*)node)->getValue());
	case (LT_FUNCTION):  return new FuncValue((AST::Function*)node);
	default: return nullptr;
	//case (LT_NULL): break;
	}
}

Value * Interpreter::interpretVariable(AST::Variable * node)
{
	string name = node->getVarId().name;
	for (int scope = currentScope(); scope >= 0; scope--)
	{
		if (_variables[scope].count(name))
			return _variables[scope][name];
	}
	throw "variable does not exist";
}

Value * Interpreter::interpretUnaryOpStatement(AST::UnaryOperationStatement * node)
{
	
	switch (node->getOperator()._type)
	{
	case (OT_RETURN):
		{
			Value* val = interpret(node->getExpression());
			Value* copy = copyValue(val);
			copy->setReturn();
			deleteIfIsTemp(val);
			return copy;
		}
	}
	return NULL;
}

Value * Interpreter::interpretVariableDeclaration(AST::VariableDeclaration * node)
{
	string type = node->getVarType().name;
	string name = node->getVarId().name;
	int scope = currentScope();
	if (_variables[scope].count(name) != 0)
		throw "illegal redefinition/multiple initialization";
	auto modifiers = node->getModifiers();
	Value* &val = _variables[scope][name];
	if (std::find_if(std::begin(modifiers), std::end(modifiers),
		[](AST::Identificator id) { return id.name == "func"; }) != std::end(modifiers))	// check if "modifiers" has "func" in it
	{
		if (type == "bool" || type == "int" || type == "frac" || type == "string" || type == "char" || type == "void")
			val = new FuncValue(type);
		else throw "custom types are not implemented yet.";
	}
	else
	{
		if		(type == "bool")	val = new BoolValue();
		else if (type == "int")		val = new IntValue();
		else if (type == "frac")	val = new FracValue();
		else if (type == "string")	val = new StringValue();
		else if (type == "char")	val = new CharValue();
		else throw "custom types are not implemented yet.";
	}
	val->setNotTemp();
	return val;
}

Value* Interpreter::interpretIfThenElse(AST::IfThenElse * node)
{
	enterBlock();
	Value* condition = interpret(node->getCondition());
	if (condition->getType() != "bool")
		throw "condition must be bool";

	Value* val;
	if (((BoolValue*)condition)->getValue())
		val = interpret(node->getThenBranch());
	else val = interpret(node->getElseBranch());

	leaveBlock();
	deleteIfIsTemp(condition);
	if (val != nullptr && val->isReturn()) return val;
	deleteIfIsTemp(val);

	return nullptr;
}

Value* Interpreter::interpretWhileLoop(AST::WhileLoop * node)
{
	enterBlock();
	Value* condition = interpret(node->getCondition());
	if (condition->getType() != "bool")
		throw "condition must be bool";
	while (((BoolValue*)condition)->getValue())
	{
		Value* val = interpret(node->getStatement());
		if (val != nullptr && val->isReturn()) { leaveBlock(); return val; }
		deleteIfIsTemp(val);
		deleteIfIsTemp(condition);
		condition = interpret(node->getCondition());
	}
	deleteIfIsTemp(condition);
	leaveBlock();
	return nullptr;
}

Value* Interpreter::interpretDoWhileLoop(AST::DoWhileLoop * node)
{
	enterBlock();
	Value* condition = nullptr;
	do {
		Value* val = interpret(node->getStatement());
		if (val != nullptr && val->isReturn()) { leaveBlock(); return val; }
		deleteIfIsTemp(val);
		deleteIfIsTemp(condition);
		condition = interpret(node->getCondition());
		if (condition->getType() != "bool")
			throw "condition must be bool";
	} while (((BoolValue*)condition)->getValue());
	leaveBlock();
	return nullptr;
}

Value * Interpreter::copyValue(Value * val)
{
	string type = val->getType();
	if		(type == "bool")	return new BoolValue(((BoolValue*)val)->getValue());
	else if (type == "int")		return new IntValue(((IntValue*)val)->getValue());
	else if (type == "frac")	return new FracValue(((FracValue*)val)->getValue());
	else if (type == "string")	return new StringValue(((StringValue*)val)->getValue());
	else if (type == "char")	return new CharValue(((CharValue*)val)->getValue());
	return nullptr;
}
