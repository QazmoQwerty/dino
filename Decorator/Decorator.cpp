#include "Decorator.h"

vector<unordered_map<unicode_string, DST::Type*, UnicodeHasherFunction>> Decorator::_variables;
unordered_map<unicode_string, DST::TypeDeclaration*, UnicodeHasherFunction> Decorator::_types;
vector<DST::Node*> Decorator::_toDelete;

#define createBasicType(name) _types[unicode_string(name)] = new DST::TypeDeclaration(unicode_string(name));

void Decorator::setup()
{
	enterBlock();
	createBasicType("type");
	createBasicType("int");
	createBasicType("bool");
	createBasicType("string");
	createBasicType("char");
	createBasicType("float");
	createBasicType("void");
}

DST::Node *Decorator::decorate(AST::Node * node)
{
	if (node == nullptr)
		return NULL;
	if (node->isExpression())
		return decorate(dynamic_cast<AST::Expression*>(node));
	else return decorate(dynamic_cast<AST::Statement*>(node));
}

DST::Expression * Decorator::decorate(AST::Expression * node)
{
	if (node == nullptr)
		return NULL;
	switch (dynamic_cast<AST::Expression*>(node)->getExpressionType())
	{
	case ET_IDENTIFIER:
		return decorate(dynamic_cast<AST::Identifier*>(node));
	case ET_BINARY_OPERATION:
		return decorate(dynamic_cast<AST::BinaryOperation*>(node));
	case ET_LITERAL:
		return decorate(dynamic_cast<AST::Literal*>(node));
	case ET_UNARY_OPERATION: 
		return decorate(dynamic_cast<AST::UnaryOperation*>(node));
	case ET_VARIABLE_DECLARATION:
		return decorate(dynamic_cast<AST::VariableDeclaration*>(node));
	case ET_ASSIGNMENT:
		return decorate(dynamic_cast<AST::Assignment*>(node));
	case ET_LIST:
		return decorate(dynamic_cast<AST::ExpressionList*>(node));
	case ET_FUNCTION_CALL:
		return decorate(dynamic_cast<AST::FunctionCall*>(node));
	default: 
		return NULL;
	}
}

DST::Statement * Decorator::decorate(AST::Statement * node)
{
	if (node == nullptr)
		return NULL;
	switch (dynamic_cast<AST::Statement*>(node)->getStatementType())
	{
	case ST_FUNCTION_CALL:
	{
		auto n = decorate(dynamic_cast<AST::FunctionCall*>(node));
		if (!n->isStatement())
			throw DinoException("expected a statement", EXT_GENERAL, node->getLine());
		return dynamic_cast<DST::ExpressionStatement*>(n);
	}
	case ST_FUNCTION_DECLARATION:
		return decorate(dynamic_cast<AST::FunctionDeclaration*>(node));
	case ST_VARIABLE_DECLARATION:
		return decorate(dynamic_cast<AST::VariableDeclaration*>(node));
	case ST_ASSIGNMENT:
		return decorate(dynamic_cast<AST::Assignment*>(node));
	case ST_STATEMENT_BLOCK:
		return decorate(dynamic_cast<AST::StatementBlock*>(node));
	case ST_PROPERTY_DECLARATION:
		return decorate(dynamic_cast<AST::PropertyDeclaration*>(node));
	case ST_IF_THEN_ELSE:
		return decorate(dynamic_cast<AST::IfThenElse*>(node));
	case ST_SWITCH:
		return decorate(dynamic_cast<AST::SwitchCase*>(node));
	case ST_FOR_LOOP:
		return decorate(dynamic_cast<AST::ForLoop*>(node));
	case ST_WHILE_LOOP: 
		return decorate(dynamic_cast<AST::WhileLoop*>(node));
	case ST_UNARY_OPERATION:
		return decorate(dynamic_cast<AST::UnaryOperationStatement*>(node));
	case ST_TYPE_DECLARATION:
		return decorate(dynamic_cast<AST::TypeDeclaration*>(node));
	case ST_DO_WHILE_LOOP: 
		return decorate(dynamic_cast<AST::DoWhileLoop*>(node));
	default: 
		return NULL;
	}
}

DST::Expression *Decorator::decorate(AST::Identifier * node)
{
	unicode_string name = node->getVarId();
	
	for (int scope = currentScope(); scope >= 0; scope--)
		if (_variables[scope].count(name))
			return new DST::Variable(node, _variables[scope][name]);
	if (_types.count(name))
		return evalType(node);
	throw DinoException("Identifier '" + name.to_string() + "' is undefined", EXT_GENERAL, node->getLine());
}

DST::FunctionDeclaration * Decorator::decorate(AST::FunctionDeclaration * node)
{
	enterBlock();
	auto decl = new DST::FunctionDeclaration(node, decorate(node->getVarDecl()));
	for (auto i : node->getParameters())
		decl->addParameter(decorate(i));
	decl->setContent(decorate(node->getContent()));
	if (decl->getContent() && !decl->getContent()->hasReturnType(decl->getReturnType()))
		throw DinoException("Not all control paths lead to a return value.", EXT_GENERAL, node->getLine());
	leaveBlock();
	auto type = new DST::FunctionType();
	type->addReturn(decl->getReturnType());	// TODO - functions that return multiple types
	for (auto i : decl->getParameters())
		type->addParameter(i->getType());
	_variables[currentScope()][decl->getVarDecl()->getVarId()] = type;
	return decl;
}

DST::PropertyDeclaration * Decorator::decorate(AST::PropertyDeclaration * node)
{
	unicode_string name = node->getVarDecl()->getVarId();
	DST::Type* type = evalType(node->getVarDecl()->getVarType());
	for (int scope = currentScope(); scope >= 0; scope--)
		if (_variables[scope].count(name))
			throw DinoException("Identifier '" + name.to_string() + "' is already in use", EXT_GENERAL, node->getLine());
	if (_types.count(name))
		throw DinoException("Identifier '" + name.to_string() + "' is a type name", EXT_GENERAL, node->getLine());

	DST::StatementBlock *get = decorate(node->getGet());

	if (get && !get->hasReturnType(type))
		throw DinoException("Not all control paths lead to a return value.", EXT_GENERAL, node->getLine());

	enterBlock();
	_variables[currentScope()][unicode_string("value")] = type;
	DST::StatementBlock *set = decorate(node->getSet());
	leaveBlock();

	_variables[currentScope()][name] = new DST::PropertyType(type, get, set);
	return new DST::PropertyDeclaration(node, get, set, type);
}

DST::UnaryOperationStatement * Decorator::decorate(AST::UnaryOperationStatement * node)
{
	return new DST::UnaryOperationStatement(node, decorate(node->getExpression()));
}

DST::TypeDeclaration * Decorator::decorate(AST::TypeDeclaration * node)
{
	enterBlock();
	auto decl = new DST::TypeDeclaration(node);
	vector<DST::Statement*> decls;

	for (auto i : node->getVariableDeclarations()) 
	{
		auto dec = decorate(i);
		decl->addDeclaration(dec, _variables[currentScope()][dec->getVarId()]);
	}
	for (auto i : node->getFunctionDeclarations()) 
	{
		auto dec = decorate(i);
		decl->addDeclaration(dec, _variables[currentScope()][dec->getVarDecl()->getVarId()]);
	}
	for (auto i : node->getPropertyDeclarations())
	{
		auto dec = decorate(i);
		decl->addDeclaration(dec, _variables[currentScope()][dec->getPropId()]);
	}

	leaveBlock();
	_types[decl->getName()] = decl;
	return decl;
}

DST::Expression * Decorator::decorate(AST::UnaryOperation * node)
{
	switch (node->getOperator()._type)
	{
		case OT_SQUARE_BRACKETS_OPEN: 
		{
			auto values = decorate(node->getExpression());
			DST::Type *prevType = NULL;
			for (auto val : ((DST::ExpressionList*)values)->getExpressions())
			{
				if (prevType && !val->getType()->equals(prevType))
					throw DinoException("Array Literal must have only one type", EXT_GENERAL, node->getLine());
				prevType = val->getType();
			}
			if (!prevType)
				throw DinoException("array literal can't be empty", EXT_GENERAL, node->getLine()); // TODO: check this one with shalev.

			return new DST::ArrayLiteral(prevType, ((DST::ExpressionList*)values)->getExpressions());
		}
		default: return NULL;
	}
}

DST::VariableDeclaration *Decorator::decorate(AST::VariableDeclaration * node)
{
	auto decl = new DST::VariableDeclaration(node);
	unicode_string name = node->getVarId();
	decl->setType(evalType(node->getVarType()));
	for (int scope = currentScope(); scope >= 0; scope--)
		if (_variables[scope].count(name))
			throw DinoException("Identifier '" + name.to_string() + "' is already in use", EXT_GENERAL, node->getLine());
	if (_types.count(name))
		throw DinoException("Identifier '" + name.to_string() + "' is a type name", EXT_GENERAL, node->getLine());
		
	_variables[currentScope()][name] = decl->getType();
	return decl;
}

DST::Assignment * Decorator::decorate(AST::Assignment * node)
{
	auto assignment = new DST::Assignment(node, decorate(node->getLeft()), decorate(node->getRight()));

	if (!assignment->getLeft()->getType()->writeable())
		throw DinoException("lvalue is read-only", EXT_GENERAL, node->getLine());
	if (!assignment->getRight()->getType()->readable())
		throw DinoException("rvalue is write-only", EXT_GENERAL, node->getLine());

	if (!assignment->getLeft()->getType()->equals(assignment->getRight()->getType()))
		throw DinoException("Assignment of different types invalid.", EXT_GENERAL, node->getLine());
	assignment->setType(assignment->getLeft()->getType());
	return assignment;
}

DST::Expression * Decorator::decorate(AST::FunctionCall * node)
{
	auto funcId = decorate(node->getFunctionId());
	DST::ExpressionList *arguments;
	if (node->getArguments() == nullptr || node->getArguments()->getExpressionType() != ET_LIST)
	{
		auto list = new DST::ExpressionList(node->getArguments());
		if (node->getArguments()) 
		{
			auto dec = decorate(node->getArguments());
			if (!dec->getType()->readable())
				throw DinoException("argument is write-only", EXT_GENERAL, node->getLine());
			list->addExpression(dec);
		}
		arguments = list;
	}
	else arguments = decorate((AST::ExpressionList*)node->getArguments());

	if (funcId->getExpressionType() == ET_TYPE)	// function type OR conversion
	{
		bool areAllTypes = true;
		for (auto i : arguments->getExpressions())
			if (i->getExpressionType() != ET_TYPE)
				areAllTypes = false;

		if (areAllTypes)
		{
			// function type
			auto type = new DST::FunctionType(node);
			type->addReturn((DST::Type*)funcId);
			for (auto i : arguments->getExpressions())
				type->addParameter((DST::Type*)i);
			return type;
		}
		if (arguments->getExpressions().size() == 1) // conversion
			return new DST::Conversion(node, (DST::Type*)funcId, arguments->getExpressions()[0]);
		throw DinoException("invalid function arguments", EXT_GENERAL, node->getLine());
	}

	auto call = new DST::FunctionCall(node, funcId, arguments);
	return call;
}

DST::FunctionLiteral * Decorator::decorate(AST::Function * node)
{
	enterBlock();
	auto lit = new DST::FunctionLiteral(node);
	auto type = new DST::FunctionType();
	if (node->getReturnType() == NULL)
		type->addReturn(new DST::BasicType(unicode_string("void")));
	else type->addReturn(evalType(node->getReturnType()));
	for (auto i : node->getParameters())
	{
		auto param = decorate(i);
		type->addParameter(param->getType());
		lit->addParameter(param);
	}
	lit->setContent(decorate(node->getContent()));
	lit->setType(type);
	if (lit->getContent() && !lit->getContent()->hasReturnType(type->getReturns()))
		throw DinoException("Not all control paths lead to a return value.", EXT_GENERAL, node->getLine());
	leaveBlock();
	return lit;
}

DST::BinaryOperation * Decorator::decorate(AST::BinaryOperation * node)
{
	if (node->getOperator()._type == OT_PERIOD) 
	{
		auto left = decorate(node->getLeft());
		auto type = left->getType();
		if (node->getRight()->getExpressionType() != ET_IDENTIFIER)
			throw DinoException("Expected an identifier", EXT_GENERAL, node->getLine());
		if (type->getExactType() == EXACT_PROPERTY && ((DST::PropertyType*)type)->hasGet())
			type = ((DST::PropertyType*)type)->getReturn();
		if (type->getExactType() != EXACT_BASIC)
			throw DinoException("Expression must have class type", EXT_GENERAL, node->getLine());
		auto memberType = _types[((DST::BasicType*)type)->getTypeId()]->getMemberType(((AST::Identifier*)node->getRight())->getVarId());
		if (memberType == nullptr)
			throw DinoException("Unkown identifier", EXT_GENERAL, node->getLine());
		//if (!(((AST::Identifier*)node->getRight())->getVarId())[0].isUpper())
		//	throw DinoException("Cannot access private member", EXT_GENERAL, node->getLine());

		auto right = new DST::Variable((AST::Identifier*)node->getRight(), memberType);
		
		auto bo = new DST::BinaryOperation(node, left, right);
		bo->setType(memberType);
		return bo;
	}

	auto bo = new DST::BinaryOperation(node, decorate(node->getLeft()), decorate(node->getRight()));


	switch (OperatorsMap::getReturnType(node->getOperator()._type))
	{
		case RT_BOOLEAN: 
			bo->setType(new DST::BasicType(CONDITION_TYPE));
			break;
		case RT_ARRAY:
			// array access
			if (bo->getLeft()->getExpressionType() == ET_IDENTIFIER)
			{
				DST::BasicType intType(unicode_string("int"));
				if (!((bo->getRight()->getExpressionType() == ET_LITERAL && ((DST::Literal*)bo->getRight())->getLiteralType() == LT_INTEGER) ||
					 (bo->getRight()->getExpressionType() == ET_IDENTIFIER && bo->getRight()->getType()->equals(&intType))))
					throw DinoException("array index must be an integer value", EXT_GENERAL, node->getLine());
				if(bo->getRight()->getExpressionType() == ET_LITERAL && ((DST::ArrayType*)bo->getLeft()->getType())->getLength() <= *((int*)((DST::Literal*)(bo->getRight()))->getValue()))
					throw DinoException("index out of range", EXT_GENERAL, node->getLine());
				bo->setType(bo->getLeft()->getType()->getType());
			}
			// array declaration
			else
			{
				if (bo->getLeft()->getExpressionType() != ET_TYPE)
					throw DinoException("expected a type", EXT_GENERAL, node->getLine());
				if (bo->getRight())
				{
					if (!(bo->getRight()->getExpressionType() == ET_LITERAL && ((DST::Literal*)bo->getRight())->getLiteralType() == LT_INTEGER))
						throw DinoException("array size must be a literal integer", EXT_GENERAL, node->getLine());
					bo->setType(new DST::ArrayType((DST::Type*)bo->getLeft(), *((int*)((DST::Literal*)(bo->getRight()))->getValue())));
				}
				else bo->setType(new DST::ArrayType((DST::Type*)bo->getLeft(), DST::UNKNOWN_ARRAY_LENGTH));
			}
			break;
		case RT_LEFT: 
			bo->setType(bo->getLeft()->getType());
			break;
		case RT_VOID: 
			throw DinoException("Could not decorate, unimplemented operator.", EXT_GENERAL, node->getLine());

		default: break;
	}

	return bo;
}

DST::Expression * Decorator::decorate(AST::Literal * node)
{
	if (node->getLiteralType() == LT_FUNCTION)
		return decorate((AST::Function*)node);
	auto lit = new DST::Literal(node);
	switch (node->getLiteralType()) 
	{
	case (LT_BOOLEAN):		lit->setType(new DST::BasicType(unicode_string("bool")));	break;
	case (LT_CHARACTER):	lit->setType(new DST::BasicType(unicode_string("char")));	break;
	case (LT_FRACTION):		lit->setType(new DST::BasicType(unicode_string("float")));	break;
	case (LT_INTEGER):		lit->setType(new DST::BasicType(unicode_string("int")));	break;
	case (LT_STRING):		lit->setType(new DST::BasicType(unicode_string("string"))); break;
	case (LT_NULL):			lit->setType(new DST::BasicType(unicode_string("null")));	break;
	default: break;
	}
	return lit;
}

DST::ExpressionList * Decorator::decorate(AST::ExpressionList * node)
{
	auto list = new DST::ExpressionList(node);
	for (auto i : node->getExpressions())
		list->addExpression(decorate(i));
	return list;
}

DST::StatementBlock * Decorator::decorate(AST::StatementBlock * node)
{
	if (!node)
		return NULL;

	enterBlock();
	auto bl = new DST::StatementBlock();
	for (auto i : dynamic_cast<AST::StatementBlock*>(node)->getStatements())
		bl->addStatement(decorate(i));
	leaveBlock();
	return bl;
}

DST::IfThenElse * Decorator::decorate(AST::IfThenElse * node)
{
	auto ite = new DST::IfThenElse(node);
	ite->setCondition(decorate(node->getCondition()));
	if (!isCondition(ite->getCondition()))
		throw DinoException("Expected a condition", EXT_GENERAL, node->getLine());
	ite->setThenBranch(decorate(node->getThenBranch()));
	ite->setElseBranch(decorate(node->getElseBranch()));
	return ite;
}

DST::SwitchCase * Decorator::decorate(AST::SwitchCase * node)
{
	auto sc = new DST::SwitchCase(node);
	sc->setExpression(decorate(node->getExpression()));
	sc->setDefault(decorate(node->getDefault()));
	for (auto c : node->getCases()) {
		sc->addCase(decorate(c._expression), decorate(c._statement));
		// if case type == swich type.
		if (!sc->getCases().back()._expression->getType()->equals(sc->getExpression()->getType()))
			throw DinoException("this constant expression has type \"" + sc->getCases().back()._expression->getType()->toShortString() + "\" instead of the required \"" + sc->getExpression()->getType()->toShortString() + "\" type", EXT_GENERAL, node->getLine());
	}
	return sc;
}

DST::ForLoop * Decorator::decorate(AST::ForLoop * node)
{
	enterBlock();
	auto loop = new DST::ForLoop(node);
	loop->setBegin(decorate(node->getBegin()));
	loop->setCondition(decorate(node->getCondition()));
	if (!isCondition(loop->getCondition()))
		throw DinoException("Expected a condition", EXT_GENERAL, node->getLine());
	loop->setIncrement(decorate(node->getIncrement()));
	loop->setStatement(decorate(node->getStatement()));
	leaveBlock();
	return loop;
}

DST::WhileLoop * Decorator::decorate(AST::WhileLoop * node)
{
	auto loop = new DST::WhileLoop(node);
	loop->setCondition(decorate(node->getCondition()));
	if (!isCondition(loop->getCondition()))
		throw DinoException("Expected a condition", EXT_GENERAL, node->getLine());
	loop->setStatement(decorate(node->getStatement()));
	return loop;
}

DST::DoWhileLoop * Decorator::decorate(AST::DoWhileLoop * node)
{
	auto loop = decorate(dynamic_cast<AST::WhileLoop*>(node));
	auto doLoop = new DST::DoWhileLoop(loop);
	return doLoop;
}

DST::Type * Decorator::evalType(AST::Expression * node)
{
	if (node->getExpressionType() == ET_IDENTIFIER)
		return new DST::BasicType(node);
	auto ret = decorate(node);
	if (ret->getExpressionType() == ET_BINARY_OPERATION)
	{
		auto arr = ret->getType();
		delete ret;
		ret = arr;
	}
	else if (ret->getExpressionType() != ET_TYPE)
		throw DinoException("expected a type", EXT_GENERAL, node->getLine());
	return (DST::Type*)ret;
}

bool Decorator::isCondition(DST::Expression * node)
{
	return node && node->getType()->getExactType() == EXACT_BASIC
		&& dynamic_cast<DST::BasicType*>(node->getType())->getTypeId() == CONDITION_TYPE;
}

void Decorator::leaveBlock()
{
	for (auto i : _variables[currentScope()])
		_toDelete.push_back(i.second);
	_variables.pop_back();
}

void Decorator::clear()
{
	leaveBlock();
	for (auto i : _toDelete)
		if (i != nullptr)
			delete i;
}
