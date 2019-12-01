#include "Decorator.h"

vector<unordered_map<unicode_string, DST::Type*, UnicodeHasherFunction>> Decorator::_variables;
unordered_map<unicode_string, DST::TypeDeclaration*, UnicodeHasherFunction> Decorator::_types;
vector<void*> Decorator::_toDelete;

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
	case ET_VARIABLE_DECLARATION:
		return decorate(dynamic_cast<AST::VariableDeclaration*>(node));
	case ET_ASSIGNMENT:
		return decorate(dynamic_cast<AST::Assignment*>(node));
	case ET_LIST:
		return decorate(dynamic_cast<AST::ExpressionList*>(node));
	case ET_FUNCTION_CALL:
		return decorate(dynamic_cast<AST::FunctionCall*>(node));
	}
	return NULL;
}

DST::Statement * Decorator::decorate(AST::Statement * node)
{
	if (node == nullptr)
		return NULL;
	switch (dynamic_cast<AST::Statement*>(node)->getStatementType())
	{
	case ST_FUNCTION_CALL:
		return decorate(dynamic_cast<AST::FunctionCall*>(node));
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
	case ST_FOR_LOOP: 
		return decorate(dynamic_cast<AST::ForLoop*>(node));
	case ST_WHILE_LOOP: 
		return decorate(dynamic_cast<AST::WhileLoop*>(node));
	case ST_UNARY_OPERATION:
		return decorate(dynamic_cast<AST::UnaryOperationStatement*>(node));
	case ST_DO_WHILE_LOOP: 
		throw DinoException("do-while loops are not implemented yet", EXT_GENERAL, node->getLine());
	}
	return NULL;
}

DST::Expression *Decorator::decorate(AST::Identifier * node)
{
	unicode_string name = node->getVarId();
	
	for (int scope = currentScope(); scope >= 0; scope--)
		if (_variables[scope].count(name))
			return new DST::Variable(node, _variables[scope][name]);
	if (_types.count(name))
		return evalType(node);
	throw DinoException::DinoException("Identifier is undefined", EXT_GENERAL, node->getLine()); // TODO: add id to error.
}

DST::FunctionDeclaration * Decorator::decorate(AST::FunctionDeclaration * node)
{
	enterBlock();
	auto decl = new DST::FunctionDeclaration(node, decorate(node->getVarDecl()));
	for (auto i : node->getParameters())
		decl->addParameter(decorate(i));
	decl->setContent(decorate(node->getContent()));
	leaveBlock();
	auto type = new DST::FunctionType();
	type->addReturn(decl->getVarDecl()->getType());	// TODO - functions that return multiple types
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
			throw DinoException::DinoException("Identifier is already in use", EXT_GENERAL, node->getLine());
	if (_types.count(name))
		throw DinoException::DinoException("Identifier is a type name", EXT_GENERAL, node->getLine());

	
	DST::StatementBlock *get = decorate(node->getGet());

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

DST::VariableDeclaration *Decorator::decorate(AST::VariableDeclaration * node)
{
	auto decl = new DST::VariableDeclaration(node);
	unicode_string name = node->getVarId();
	decl->setType(evalType(node->getVarType()));
	for (int scope = currentScope(); scope >= 0; scope--)
		if (_variables[scope].count(name))
			throw DinoException::DinoException("Identifier is already in use", EXT_GENERAL, node->getLine());
	if (_types.count(name))
		throw DinoException::DinoException("Identifier is a type name", EXT_GENERAL, node->getLine());
		
	_variables[currentScope()][name] = decl->getType();
	return decl;
}

DST::Assignment * Decorator::decorate(AST::Assignment * node)
{
	auto assignment = new DST::Assignment(node, decorate(node->getLeft()), decorate(node->getRight()));
	if (!assignment->getLeft()->getType()->equals(assignment->getRight()->getType()))
		throw DinoException("Assignment of different types invalid.", EXT_GENERAL, node->getLine());
	assignment->setType(assignment->getLeft()->getType());
	return assignment;
}

DST::FunctionCall * Decorator::decorate(AST::FunctionCall * node)
{
	auto call = new DST::FunctionCall(node);
	call->setFunctionId(decorate(node->getFunctionId()));
	if (node->getArguments()->getExpressionType() != ET_LIST)
	{
		auto list = new DST::ExpressionList(node->getArguments());
		list->addExpression(decorate(node->getArguments()));
		call->setArguments(list);
	}
	else call->setArguments(decorate((AST::ExpressionList*)node->getArguments()));

	return call;
}

DST::BinaryOperation * Decorator::decorate(AST::BinaryOperation * node)
{
	auto bo = new DST::BinaryOperation(node, decorate(node->getLeft()), decorate(node->getRight()));

	switch (OperatorsMap::getReturnType(node->getOperator()._type))
	{
		case RT_BOOLEAN: 
			bo->setType(new DST::BasicType(CONDITION_TYPE));
			break;
		case RT_LEFT: 
			bo->setType(bo->getLeft()->getType());
			break;
		case RT_VOID: 
			throw DinoException("Could not decorate, unimplemented operator.", EXT_GENERAL, node->getLine());
	}

	return bo;
}

DST::Literal * Decorator::decorate(AST::Literal * node)
{
	auto lit = new DST::Literal(node);
	switch (node->getLiteralType()) 
	{
	case (LT_BOOLEAN):		lit->setType(new DST::BasicType(unicode_string("bool")));	break;
	case (LT_CHARACTER):	lit->setType(new DST::BasicType(unicode_string("char")));	break;
	case (LT_FRACTION):		lit->setType(new DST::BasicType(unicode_string("float")));	break;
	case (LT_INTEGER):		lit->setType(new DST::BasicType(unicode_string("int")));	break;
	case (LT_STRING):		lit->setType(new DST::BasicType(unicode_string("string"))); break;
	case (LT_NULL):			lit->setType(new DST::BasicType(unicode_string("null")));	break;
	case (LT_FUNCTION):		throw DinoException("function literals are not implemented yet", EXT_GENERAL, node->getLine());
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

DST::Type * Decorator::evalType(AST::Expression * node)
{
	if (node->getExpressionType() == ET_IDENTIFIER)
		return new DST::BasicType(node);
	return nullptr;
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
