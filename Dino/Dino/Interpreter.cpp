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
			case(ST_TYPE_DECLARATION):
				interpretTypeDeclaration(dynamic_cast<AST::TypeDeclaration*>(node));
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
		auto expression = dynamic_cast<AST::Expression*>(node);
		switch (expression->getExpressionType())
		{
		case (ET_BINARY_OPERATION):
			return interpretBinaryOp(dynamic_cast<AST::BinaryOperation*>(node));
		case (ET_UNARY_OPERATION):
			return interpretUnaryOp(dynamic_cast<AST::UnaryOperation*>(node));
		case (ET_CONDITIONAL_EXPRESSION):
			throw "conditional expression are not supported yet";
		case (ET_LITERAL):
			return interpretLiteral(dynamic_cast<AST::Literal*>(node));
		case (ET_VARIABLE):
			return interpretVariable(dynamic_cast<AST::Variable*>(node));
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
			else if (type == "ptr") ((PtrValue*)lvalue)->setValue(((PtrValue*)rvalue)->getValue());
			//else throw "type does not exist";
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
	Value* ret = nullptr;
	Value* leftVal = interpret(node->getLeft());

	if (node->getOperator()._type == OT_PERIOD)
	{
		Value* left = nullptr;
		if (leftVal->getType() == "ptr")
			left = ((PtrValue*)leftVal)->getValue();
		else left = leftVal;
		if (_types.count(left->getType()) == 0)
			throw "type doesn't exist";
		if (node->getRight()->getExpressionType() != ET_VARIABLE)
			throw "right of '.' operator must be a variable name";
		string varName = dynamic_cast<AST::Variable*>(node->getRight())->getVarId().name;
		return ((TypeValue*)left)->getVariable(varName);
	}

	Value* rightVal = interpret(node->getRight());
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
		for (unsigned int i = 0; i < node->getParameters().size(); i++) 
		{
			Value* val = interpret(node->getParameters()[i]);
			std::cout << val->toString();
			if (i != node->getParameters().size() - 1) std::cout << " ";
		}
		std::cout << std::endl;
		return NULL;
	}
	enterBlock();
	Value* thisPtr = nullptr;
	Value* val = nullptr;
	if (node->getFunctionId()->getExpressionType() == ET_BINARY_OPERATION &&
		dynamic_cast<AST::BinaryOperation*>(node->getFunctionId())->getOperator()._type == OT_PERIOD)
	{
		auto tempNode = dynamic_cast<AST::BinaryOperation*>(node->getFunctionId());
		thisPtr = interpret(tempNode->getLeft());

		Value* left = nullptr;
		if (thisPtr->getType() == "ptr")
			left = ((PtrValue*)thisPtr)->getValue();
		else left = thisPtr;
		if (_types.count(left->getType()) == 0)
			throw "type doesn't exist";
		if (tempNode->getRight()->getExpressionType() != ET_VARIABLE)
			throw "right of '.' operator must be a variable name";
		string varName = dynamic_cast<AST::Variable*>(tempNode->getRight())->getVarId().name;
		val = ((TypeValue*)left)->getVariable(varName);
		//Value* leftTemp = interpret
	}
	else val = interpret(node->getFunctionId());
	if (val->getType() != "func")
		throw "cannot invoke non-function!";
	auto func = ((FuncValue*)val);
	auto values = node->getParameters();
	auto params = func->getValue()->getParameters();

	bool isMethod = false;
	if (params.size() != 0 && params[0]->getVarId().name == "this")
	{
		isMethod = true;
		if (params.size() != values.size() + 1)
			throw "Incorrect number of parameter inputs";
	}
	else if (params.size() != values.size())
		throw "Incorrect number of parameter inputs";

	for (unsigned int i = 0; i < params.size(); i++)
	{
		Value* rvalue = nullptr;
		if (i == 0 && isMethod)
			rvalue = thisPtr;
		else rvalue = interpret(values[isMethod ? i - 1 : i]);
		Value* lvalue = interpret(params[i]);

		string name = params[i]->getVarId().name;
		if (lvalue->getType() != rvalue->getType())
			throw "Incompatible function inputs";
		
		string type = lvalue->getType();
		if (type == "int")			((IntValue*)lvalue)->setValue(((IntValue*)rvalue)->getValue());
		else if (type == "bool")	((BoolValue*)lvalue)->setValue(((BoolValue*)rvalue)->getValue());
		else if (type == "frac")	((FracValue*)lvalue)->setValue(((FracValue*)rvalue)->getValue());
		else if (type == "string")  ((StringValue*)lvalue)->setValue(((StringValue*)rvalue)->getValue());
		else if (type == "char")	((CharValue*)lvalue)->setValue(((CharValue*)rvalue)->getValue());
		else if (type == "func")	((FuncValue*)lvalue)->setValue(((FuncValue*)rvalue)->getValue());
		else if (type == "ptr") 
		{
			((PtrValue*)lvalue)->setValue(((PtrValue*)rvalue)->getValue());
		}
		else if (_types.count(type))
		{
			//std::cout << "warning: garbage collection does not exist yet!" << std::endl;
			//lvalue = rvalue;
		}
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
		if (_variables[scope].count(name))
			return _variables[scope][name];

	for (int scope = currentScope(); scope >= 0; scope--)
	{
		if (_variables[scope].count("this"))
		{
			try { return ((TypeValue*)((PtrValue*)_variables[scope]["this"])->getValue())->getVariable(name); }
			catch (exception)  { throw "variable does not exist"; }
		}
	}

	throw "variable does not exist";
}

void Interpreter::interpretTypeDeclaration(AST::TypeDeclaration * node)
{
	TypeDefinition typeDef;
	typeDef._name = node->getName().name;
	typeDef._variables = unordered_map<string, VariableTypeDefinition>();
	for (auto decl : node->getVariableDeclarations())
	{
		VariableTypeDefinition vtd;
		for (auto modifier : decl->getModifiers())
			vtd.modifiers.push_back(modifier.name);
		vtd.type = decl->getVarType().name;
		//string varName = decl->getVarId().name;
		typeDef._variables[decl->getVarId().name] = vtd;
	}
	for (auto funcDecl : node->getFunctionDeclarations())
	{
		if (funcDecl->getLeft()->getExpressionType() != ET_VARIABLE_DECLARATION)
			throw "TODO - proper error";
		auto varDecl = dynamic_cast<AST::VariableDeclaration*>(funcDecl->getLeft());
		string funcName = varDecl->getVarId().name;
		if (funcDecl->getRight()->getExpressionType() != ET_LITERAL ||
			dynamic_cast<AST::Literal*>(funcDecl->getRight())->getLiteralType() != LT_FUNCTION)
			throw "cannot assign non function value to function";
		auto funcLit = (AST::Function*)funcDecl->getRight();
		auto thisVarDecl = new AST::VariableDeclaration();
		thisVarDecl->setType({ typeDef._name });
		thisVarDecl->setVarId({ "this" });
		funcLit->addParameterToStart(thisVarDecl);
		typeDef._functions[funcName] = new FuncValue(funcLit);
	}
	_types[typeDef._name] = typeDef;
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
		if (type == "bool" || type == "int" || type == "frac" || type == "string" || type == "char" || type == "void" || _types.count(type))
			val = new FuncValue(type);
		else throw "type does not exist";
		//else throw "custom types are not implemented yet.";
	}
	else
	{
		if		(type == "bool")	val = new BoolValue();
		else if (type == "int")		val = new IntValue();
		else if (type == "frac")	val = new FracValue();
		else if (type == "string")	val = new StringValue();
		else if (type == "char")	val = new CharValue();
		else if (_types.count(type)) val = new PtrValue(type, new TypeValue(type, _types));
		//else if (_types.count(type)) val = new TypeValue(type, _types);
		else throw "type does not exist";
	}
	val->setNotTemp();
	if (val->getType() == "ptr" && ((PtrValue*)val)->getValue())
		((PtrValue*)val)->getValue()->setNotTemp();
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
	else if (type == "ptr")		return new PtrValue(((PtrValue*)val)->getPtrType(), ((PtrValue*)val)->getValue());
	else throw "cannot copy type that does not exist";
}

TypeValue::TypeValue(string typeName, unordered_map<string, TypeDefinition> &types) : Value(typeName), _types(types)
{
	_variables = unordered_map<string, Value*>();
	if (_types.count(typeName) == 0)
		throw "nonexistant type";
	_typeDefinition = _types[typeName];
	for (auto i : _typeDefinition._variables)
	{
		Value* var = nullptr;
		if (i.second.type == "int") var = new IntValue();
		else if (i.second.type == "bool") var = new BoolValue();
		else if (i.second.type == "char") var = new CharValue();
		else if (i.second.type == "frac") var = new FracValue();
		else if (i.second.type == "string") var = new StringValue();
		else if (_types.count(i.second.type)) 
			var = new TypeValue(i.second.type, _types);
		else throw "nonexistant type";
		var->setNotTemp();
		_variables[i.first] = var;
	}
}

/*void TypeValue::setVariable(string name, Value * val) // no public/private modifiers yet
{
	if (_variables.count(name) == 0)
		throw "variable does not exist";
	Value* v = _variables[name];
	string type = val->getType();
	if (type != v->getType())
		throw "incompatible "
	//if (type == "bool")	BoolValue(((BoolValue*)val)->getValue());
	//else if (type == "int")		return new IntValue(((IntValue*)val)->getValue());
	//else if (type == "frac")	return new FracValue(((FracValue*)val)->getValue());
	//else if (type == "string")	return new StringValue(((StringValue*)val)->getValue());
	//else if (type == "char")	return new CharValue(((CharValue*)val)->getValue());
}*/

Value * TypeValue::getVariable(string name)	// no public/private modifiers yet
{
	if (_typeDefinition._functions.count(name))
		return _typeDefinition._functions[name];
	if (_variables.count(name))
		return _variables[name];
	throw "variable does not exist";
}

bool TypeValue::hasVariable(string name)
{
	return _typeDefinition._functions.count(name) || _variables.count(name);
}
