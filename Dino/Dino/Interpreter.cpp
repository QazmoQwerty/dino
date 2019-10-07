#include "Interpreter.h"

#define deleteIfIsTemp(n) { if (n != nullptr && n->isTemp()) delete n; }

void Interpreter::leaveBlock()
{
	for (auto i : _variables[currentScope()])
		delete i.second;
	_variables.pop_back();
}

Value * Interpreter::evaluateProperty(Value * val)
{
	PropertyValue* pv = (PropertyValue*)val;
	if (pv->getGet() == nullptr)
		throw "property has no getter";

	if (pv->getThisPtr() == nullptr) {
		Value* ret = interpret(pv->getGet());
		if (ret && ret->isReturn())
			return ret;
		else throw "getter must return a value!";
	}
	
	enterBlock();
	_variables[currentScope()]["this"] = pv->getThisPtr();
	_currentMinScope.push(currentScope());
	_currentNamespace.push(((PtrValue*)pv->getThisPtr())->getPtrType());	// TODO - make this safe
	Value* ret = interpret(pv->getGet());
	_currentNamespace.pop();
	_currentMinScope.pop();
	leaveBlock();
	if (ret && ret->isReturn())
		return ret;
	else throw "getter must return a value!";
}

Value * Interpreter::callSetter(PropertyValue * lvalue, Value * rvalue)
{

	if (lvalue->getSet() == nullptr)
		throw "property has no setter";
	enterBlock();
	Value* rcopy = copyValue(rvalue);
	rcopy->setNotTemp();
	_variables[currentScope()]["value"] = rcopy;
	if (lvalue->getThisPtr() == nullptr) {
		Value* ret = interpret(lvalue->getSet());
		if (ret && ret->isReturn()) 
		{
			leaveBlock();
			return ret;
		}
		else throw "getter must return a value!";
	}

	_variables[currentScope()]["this"] = lvalue->getThisPtr();
	_currentMinScope.push(currentScope());
	_currentNamespace.push(((PtrValue*)lvalue->getThisPtr())->getPtrType());	// TODO - make this safe
	Value* ret = interpret(lvalue->getSet());
	_currentNamespace.pop();
	_currentMinScope.pop();
	leaveBlock();
	return copyValue(rvalue);
	return nullptr;
}

Value * Interpreter::copyValue(Value * val)
{
	string type = val->getType();
	if (type == "bool")	return new BoolValue(((BoolValue*)val)->getValue());
	else if (type == "int")		return new IntValue(((IntValue*)val)->getValue());
	else if (type == "frac")	return new FracValue(((FracValue*)val)->getValue());
	else if (type == "string")	return new StringValue(((StringValue*)val)->getValue());
	else if (type == "char")	return new CharValue(((CharValue*)val)->getValue());
	else if (type == "ptr")		return new PtrValue(((PtrValue*)val)->getPtrType(), ((PtrValue*)val)->getValue());
	else if (type == "func")	return new FuncValue(((FuncValue*)val)->getValue());
	else throw "cannot copy type that does not exist";
}

Value* Interpreter::interpret(AST::Node * node)
{
	if (node == nullptr)
		return NULL;

	if (node->isStatement()) switch (dynamic_cast<AST::Statement*>(node)->getStatementType())
	{
		case(ST_ASSIGNMENT):				return interpretAssignment(dynamic_cast<AST::Assignment*>(node));
		case(ST_VARIABLE_DECLARATION):		return interpretVariableDeclaration(dynamic_cast<AST::VariableDeclaration*>(node));
		case(ST_TYPE_DECLARATION):			return interpretTypeDeclaration(dynamic_cast<AST::TypeDeclaration*>(node));
		case (ST_FUNCTION_CALL):			return interpretFuncCall(dynamic_cast<AST::FunctionCall*>(node));
		case(ST_IF_THEN_ELSE):				return interpretIfThenElse(dynamic_cast<AST::IfThenElse*>(node));
		case(ST_STATEMENT_BLOCK):			return interpretStatementBlock(dynamic_cast<AST::StatementBlock*>(node));
		case(ST_WHILE_LOOP):				return interpretWhileLoop(dynamic_cast<AST::WhileLoop*>(node));
		case(ST_INCREMENT):					return interpretIncrement(dynamic_cast<AST::Increment*>(node));
		case(ST_DO_WHILE_LOOP):				return interpretDoWhileLoop(dynamic_cast<AST::DoWhileLoop*>(node));
		case(ST_UNARY_OPERATION):			return interpretUnaryOpStatement(dynamic_cast<AST::UnaryOperationStatement*>(node));
	}
	else switch (dynamic_cast<AST::Expression*>(node)->getExpressionType())
	{
		case (ET_BINARY_OPERATION):			return interpretBinaryOp(dynamic_cast<AST::BinaryOperation*>(node));
		case (ET_UNARY_OPERATION):			return interpretUnaryOp(dynamic_cast<AST::UnaryOperation*>(node));
		case (ET_CONDITIONAL_EXPRESSION):	throw "conditional expression are not supported yet";
		case (ET_LITERAL):					return interpretLiteral(dynamic_cast<AST::Literal*>(node));
		case (ET_VARIABLE):					return interpretVariable(dynamic_cast<AST::Variable*>(node));
	}

	return NULL;
}

#define defineEqualAOP(valType) (((valType*)lvalue)->setValue(((valType*)rvalue)->getValue()))
#define defineAOP(valType, op) (((valType*)lvalue)->setValue(((valType*)lvalue)->getValue() op ((valType*)rvalue)->getValue()))

Value * Interpreter::interpretAssignment(AST::Assignment * node)
{
	if (!OperatorsMap::isAssignment(node->getOperator()._type))
		throw "Operator is not assignment!";
	Value* rvalue = interpret(node->getRight());
	Value* lvalue = interpret(node->getLeft());

	if (lvalue == nullptr)
		throw "left of assignment must be a value";

	string type = lvalue->getType();

	if (rvalue == nullptr)
		throw "right of assignment must be a value";

	if (rvalue->getType() == "property")
		rvalue = evaluateProperty(rvalue);

	if (lvalue->getType() == "property")
	{
		//throw "property lvalues are not supported at the moment.";
		if (node->getOperator()._type == OT_ASSIGN_EQUAL)
			return callSetter((PropertyValue*)lvalue, rvalue);
		else throw "only = assignment operator is currently supported for property lvalues.";
	}

	if (type != rvalue->getType())
		throw "different types invalid";

	switch (node->getOperator()._type)
	{
		case (OT_ASSIGN_EQUAL):
			if (type == "int")		defineEqualAOP(IntValue);
			else if (type == "bool")	defineEqualAOP(BoolValue);
			else if (type == "frac")	defineEqualAOP(FracValue);
			else if (type == "string")	defineEqualAOP(StringValue);
			else if (type == "char")	defineEqualAOP(CharValue);
			else if (type == "func")	defineEqualAOP(FuncValue);
			else if (type == "ptr")		defineEqualAOP(PtrValue);
			break;
		case (OT_ASSIGN_ADD):
			if (type == "int")		defineAOP(IntValue, +);
			else if (type == "frac")	defineAOP(FracValue, +);
			else if (type == "string")	defineAOP(StringValue, +);
			break;
		case (OT_ASSIGN_SUBTRACT):
			if (type == "int")		defineAOP(IntValue, -);
			else if (type == "frac")	defineAOP(FracValue, -);
			break;
		case (OT_ASSIGN_MULTIPLY):
			if (type == "int")		defineAOP(IntValue, *);
			else if (type == "frac")	defineAOP(FracValue, *);
			break;
		case (OT_ASSIGN_DIVIDE):
			if (type == "int")		defineAOP(IntValue, / );
			else if (type == "frac")	defineAOP(FracValue, / );
			break;
		case (OT_ASSIGN_MODULUS):
			if (type == "int") ((IntValue*)lvalue)->setValue(((IntValue*)lvalue)->getValue() % ((IntValue*)rvalue)->getValue());
			break;
	}
	deleteIfIsTemp(rvalue);
	return copyValue(lvalue);
	return nullptr;
}

#define defineBOP(retType, argType, op) (new retType(((argType*)leftVal)->getValue() op ((argType*)rightVal)->getValue()))

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
		if (left == nullptr)
			throw "nullptr exception";
		if (_types.count(left->getType()) == 0)
			throw "type doesn't exist";
		if (node->getRight()->getExpressionType() != ET_VARIABLE)
			throw "right of '.' operator must be a variable name";
		string varName = dynamic_cast<AST::Variable*>(node->getRight())->getVarId().name;
		Value* ret = ((TypeValue*)left)->getVariable(varName, _currentNamespace.top());
		if (ret->getType() == "property")
			((PropertyValue*)ret)->setThisPtr(copyValue(leftVal));	// set thisPtr to point at leftVal (the pointer to the left hand expression)
		return ret;
	}

	Value* rightVal = interpret(node->getRight());

	if (rightVal->getType() == "property")
		rightVal = evaluateProperty(rightVal);
	if (leftVal->getType() == "property")
		leftVal = evaluateProperty(leftVal);

	if (leftVal->getType() != rightVal->getType())
		throw "different types invalid";
	string type = leftVal->getType();
	switch (node->getOperator()._type)
	{
		case (OT_EQUAL):
			if		(type == "int")		ret = defineBOP(BoolValue, IntValue, ==);
			else if (type == "bool")	ret = defineBOP(BoolValue, BoolValue, == );
			else if (type == "frac")	ret = defineBOP(BoolValue, FracValue, == );
			else if (type == "char")	ret = defineBOP(BoolValue, CharValue, == );
			else if (type == "string")	ret = defineBOP(BoolValue, StringValue, == );
			else if (type == "ptr")		ret = defineBOP(BoolValue, PtrValue, == );
			break;
		case (OT_NOT_EQUAL):
			if		(type == "int")		ret = defineBOP(BoolValue, IntValue, != );
			else if (type == "bool")	ret = defineBOP(BoolValue, BoolValue, != );
			else if (type == "frac")	ret = defineBOP(BoolValue, FracValue, != );
			else if (type == "char")	ret = defineBOP(BoolValue, CharValue, != );
			else if (type == "string")	ret = defineBOP(BoolValue, StringValue, != );
			else if (type == "ptr")		ret = defineBOP(BoolValue, PtrValue, != );
			break;
		case (OT_SMALLER):
			if		(type == "int")		ret = defineBOP(BoolValue, IntValue, < );
			else if (type == "frac")	ret = defineBOP(BoolValue, FracValue, < );
			else if (type == "char")	ret = defineBOP(BoolValue, CharValue, < );
			else if (type == "string")	ret = defineBOP(BoolValue, StringValue, < );
			break;
		case (OT_SMALLER_EQUAL):
			if		(type == "int")		ret = defineBOP(BoolValue, IntValue, <= );
			else if (type == "frac")	ret = defineBOP(BoolValue, FracValue, <= );
			else if (type == "char")	ret = defineBOP(BoolValue, CharValue, <= );
			else if (type == "string")	ret = defineBOP(BoolValue, StringValue, <= );
			break;
		case (OT_GREATER):
			if		(type == "int")		ret = defineBOP(BoolValue, IntValue, > );
			else if (type == "frac")	ret = defineBOP(BoolValue, FracValue, > );
			else if (type == "char")	ret = defineBOP(BoolValue, CharValue, > );
			else if (type == "string")	ret = defineBOP(BoolValue, StringValue, > );
			break;
		case (OT_GREATER_EQUAL):
			if		(type == "int")		ret = defineBOP(BoolValue, IntValue, >= );
			else if (type == "frac")	ret = defineBOP(BoolValue, FracValue, >= );
			else if (type == "char")	ret = defineBOP(BoolValue, CharValue, >= );
			else if (type == "string")	ret = defineBOP(BoolValue, StringValue, >= );
			break;
		case (OT_LOGICAL_AND):
			if		(type == "bool")	ret = defineBOP(BoolValue, BoolValue, && );
			break;
		case (OT_LOGICAL_OR):
			if		(type == "bool")	ret = defineBOP(BoolValue, BoolValue, ||);
			break;
		case(OT_ADD):
			if		(type == "int")		ret = defineBOP(IntValue, IntValue, + );
			else if (type == "frac")	ret = defineBOP(FracValue, FracValue, + );
			else if (type == "string")	ret = defineBOP(StringValue, StringValue, + );
			break;
		case(OT_SUBTRACT):
			if		(type == "int")		ret = defineBOP(IntValue, IntValue, - );
			else if (type == "frac")	ret = defineBOP(FracValue, FracValue, - );
			break;
		case(OT_MULTIPLY):
			if		(type == "int")		ret = defineBOP(IntValue, IntValue, * );
			else if (type == "frac")	ret = defineBOP(FracValue, FracValue, * );
			break;
		case(OT_DIVIDE):
			if		(type == "int")		ret = defineBOP(IntValue, IntValue, / );
			else if (type == "frac")	ret = defineBOP(FracValue, FracValue, / );
			break;
		case(OT_MODULUS):
			if		(type == "int")		ret = defineBOP(IntValue, IntValue, % );
			break;
		default: throw "unsupported binary operator";
	}
	deleteIfIsTemp(leftVal);
	deleteIfIsTemp(rightVal);
	if (ret == nullptr)
		throw "unsupported binary operator";
	return ret;
}

Value * Interpreter::interpretUnaryOp(AST::UnaryOperation * node)
{
	if (node->getOperator()._type == OT_NEW && node->getExpression()->getExpressionType() == ET_VARIABLE)
	{
		TypeValue* t = new TypeValue(dynamic_cast<AST::Variable*>(node->getExpression())->getVarId().name, _types);
		t->setNotTemp();
		return new PtrValue(t->getType(), t);
	}

	Value* val = interpret(node->getExpression());

	if (val->getType() == "property")
		val = evaluateProperty(val);

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
			if (val->getType() == "property")
				val = evaluateProperty(val);
			std::cout << val->toString();
			if (i != node->getParameters().size() - 1) std::cout << " ";
		}
		std::cout << std::endl;
		return NULL;
	}
	if (node->getFunctionId()->getExpressionType() == ET_VARIABLE
		&& ((AST::Variable*)node->getFunctionId())->getVarId().name == "PrintL")
	{
		for (unsigned int i = 0; i < node->getParameters().size(); i++)
		{
			Value* val = interpret(node->getParameters()[i]);
			if (val->getType() == "property")
				val = evaluateProperty(val);
			std::cout << val->toString();
			if (i != node->getParameters().size() - 1) std::cout << " ";
		}
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
		val = ((TypeValue*)left)->getVariable(varName, _currentNamespace.top());
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
		{
			_currentNamespace.push(((PtrValue*)thisPtr)->getPtrType());
			rvalue = thisPtr;
		}
		else rvalue = interpret(values[isMethod ? i - 1 : i]);
		Value* lvalue = interpret(params[i]);

		string name = params[i]->getVarId().name;

		if (rvalue->getType() == "property")
			rvalue = evaluateProperty(rvalue);

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
		else throw "custom types are not supported yet";
	}
	_currentMinScope.push(currentScope());
	Value* ret = interpret(func->getValue()->getContent());
	leaveBlock();
	_currentMinScope.pop();
	if (isMethod) _currentNamespace.pop();
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
		case (LT_NULL):		 return new PtrValue("null", NULL);
		default:			 return nullptr;
	}
}

Value * Interpreter::interpretVariable(AST::Variable * node)
{
	string name = node->getVarId().name;

	for (int scope = currentScope(); scope >= _currentMinScope.top(); scope--)
		if (_variables[scope].count(name))
			return _variables[scope][name];
	if (_currentMinScope.top() != 0 && _variables[0].count(name))
		return _variables[0][name];	// _variables[0] is for global variables

	for (int scope = currentScope(); scope >= _currentMinScope.top(); scope--)
	{
		if (_variables[scope].count("this"))
		{
			try { return ((TypeValue*)((PtrValue*)_variables[scope]["this"])->getValue())->getVariable(name, _currentNamespace.top()); }
			catch (exception)  { throw "variable does not exist"; }
		}
	}
	throw "variable does not exist";
}

Value* Interpreter::interpretTypeDeclaration(AST::TypeDeclaration * node)
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
		typeDef._variables[decl->getVarId().name] = vtd;
	}
	for (auto propDecl : node->getPropertyDeclarations())
	{
		PropertyDefinition propDef;
		for (auto modifier : propDecl->getVarDecl()->getModifiers())
			propDef.modifiers.push_back(modifier.name);
		propDef.value = new PropertyValue(propDecl->getSet(), propDecl->getGet(), propDecl->getVarDecl()->getVarType().name);
		typeDef._properties[propDecl->getVarDecl()->getVarId().name] = propDef;
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
		for (auto modifier : varDecl->getModifiers())
			typeDef._functions[funcName].modifiers.push_back(modifier.name);
		typeDef._functions[funcName].value = new FuncValue(funcLit);
	}
	_types[typeDef._name] = typeDef;
	return nullptr;
}

Value * Interpreter::interpretUnaryOpStatement(AST::UnaryOperationStatement* node)
{
	Value* val = interpret(node->getExpression());

	if (val->getType() == "property")
		val = evaluateProperty(val);

	switch (node->getOperator()._type)
	{
		case (OT_RETURN):
		{
			Value* copy = copyValue(val);
			copy->setReturn();
			deleteIfIsTemp(val);
			return copy;
		}
		case(OT_DELETE):
		{	
			if (val->getType() == "ptr") 
			{
				delete ((PtrValue*)val)->getValue();
				((PtrValue*)val)->setValue(NULL);
			}
			else throw "cannot delete non-pointer value";
		}
	}
	return NULL;
}

Value * Interpreter::interpretVariableDeclaration(AST::VariableDeclaration* node)
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
	}
	else
	{
		if		(type == "bool")	val = new BoolValue();
		else if (type == "int")		val = new IntValue();
		else if (type == "frac")	val = new FracValue();
		else if (type == "string")	val = new StringValue();
		else if (type == "char")	val = new CharValue();
		else if (_types.count(type)) val = new PtrValue(type, NULL);
		else throw "type does not exist";
	}
	val->setNotTemp();
	if (val->getType() == "ptr" && ((PtrValue*)val)->getValue())
		((PtrValue*)val)->getValue()->setNotTemp();
	return val;
}

Value* Interpreter::interpretIfThenElse(AST::IfThenElse* node)
{
	//enterBlock();
	Value* condition = interpret(node->getCondition());
	if (condition->getType() != "bool")
		throw "condition must be bool";

	Value* val;
	if (((BoolValue*)condition)->getValue())
		val = interpret(node->getThenBranch());
	else val = interpret(node->getElseBranch());

	//leaveBlock();
	deleteIfIsTemp(condition);
	if (val != nullptr && val->isReturn()) return val;
	deleteIfIsTemp(val);

	return nullptr;
}

Value* Interpreter::interpretWhileLoop(AST::WhileLoop* node)
{
	//enterBlock();
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
	//leaveBlock();
	return nullptr;
}

Value* Interpreter::interpretDoWhileLoop(AST::DoWhileLoop* node)
{
	//enterBlock();
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
	//leaveBlock();
	return nullptr;
}

Value * Interpreter::interpretStatementBlock(AST::StatementBlock * node)
{
	enterBlock();
	for (auto i : (dynamic_cast<AST::StatementBlock*>(node))->getChildren())
	{
		Value* val = interpret(i);
		if (val != nullptr && val->isReturn()) 
		{
			leaveBlock();
			return val;
		}
	}
	leaveBlock();
	return nullptr;
}
