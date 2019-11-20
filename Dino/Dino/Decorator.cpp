#include "Decorator.h"

unordered_map<unicode_string, DST::Type*, UnicodeHasherFunction> Decorator::_variables;
unordered_map<unicode_string, DST::TypeDeclaration*, UnicodeHasherFunction> Decorator::_types;

#define createBasicType(name) _types[unicode_string(name)] = new DST::TypeDeclaration(unicode_string(name));

void Decorator::setup()
{
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
	switch (dynamic_cast<AST::Expression*>(node)->getExpressionType())
	{
	case ET_VARIABLE:
		return decorate(dynamic_cast<AST::Variable*>(node));
	case ET_BINARY_OPERATION:
		return decorate(dynamic_cast<AST::BinaryOperation*>(node));
	case ET_LITERAL:
		return decorate(dynamic_cast<AST::Literal*>(node));
	case ET_VARIABLE_DECLARATION:
		return decorate(dynamic_cast<AST::VariableDeclaration*>(node));
	case ET_ASSIGNMENT:
		return decorate(dynamic_cast<AST::Assignment*>(node));
	}
	return NULL;
}

DST::Statement * Decorator::decorate(AST::Statement * node)
{
	switch (dynamic_cast<AST::Statement*>(node)->getStatementType())
	{
	case ST_VARIABLE_DECLARATION:
		return decorate(dynamic_cast<AST::VariableDeclaration*>(node));
	case ST_ASSIGNMENT:
		return decorate(dynamic_cast<AST::Assignment*>(node));
	case ST_STATEMENT_BLOCK:
		return decorate(dynamic_cast<AST::StatementBlock*>(node));
	case ST_IF_THEN_ELSE:
		return decorate(dynamic_cast<AST::IfThenElse*>(node));
	case ST_FOR_LOOP: 
		return decorate(dynamic_cast<AST::ForLoop*>(node));
	case ST_WHILE_LOOP: 
		return decorate(dynamic_cast<AST::WhileLoop*>(node));
	case ST_DO_WHILE_LOOP: 
		throw DinoException("do-while loops are not implemented yet", EXT_GENERAL, node->getLine());
	}
	return NULL;
}

DST::Expression *Decorator::decorate(AST::Variable * node)
{
	if (_variables.count(node->getVarId()))
		return new DST::Variable(node, _variables[node->getVarId()]);

	else if (_types.count(node->getVarId()))
		return evalType(node);

	else throw DinoException::DinoException("Identifier is undefined", EXT_GENERAL, node->getLine()); // TODO: add id to error.
}

DST::VariableDeclaration *Decorator::decorate(AST::VariableDeclaration * node)
{
	auto decl = new DST::VariableDeclaration(node);
	decl->setType(evalType(node->getVarType()));
	if (_variables.count(node->getVarId()))
		throw DinoException::DinoException("Identifier is already in use", EXT_GENERAL, node->getLine());
	_variables[node->getVarId()] = decl->getType();
	return decl;
}

DST::Assignment * Decorator::decorate(AST::Assignment * node)
{
	auto assignment = new DST::Assignment(node, decorate(node->getLeft()), decorate(node->getRight()));
	assignment->setType(assignment->getLeft()->getType());
	return assignment;
}

DST::BinaryOperation * Decorator::decorate(AST::BinaryOperation * node)
{
	auto bo = new DST::BinaryOperation(node, decorate(node->getLeft()), decorate(node->getRight()));

	// TODO - determine type
	if (node->getOperator()._type == OT_EQUAL || node->getOperator()._type == OT_SMALLER || node->getOperator()._type == OT_LOGICAL_NOT || node->getOperator()._type == OT_LOGICAL_AND)
		bo->setType(new DST::BasicType(CONDITION_TYPE)); // temporary
	else bo->setType(bo->getLeft()->getType());	// temporary

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

DST::StatementBlock * Decorator::decorate(AST::StatementBlock * node)
{
	auto bl = new DST::StatementBlock();
	for (auto i : dynamic_cast<AST::StatementBlock*>(node)->getStatements())
		bl->addStatement(decorate(i));
	return bl;
}

DST::IfThenElse * Decorator::decorate(AST::IfThenElse * node)
{
	auto ite = new DST::IfThenElse(node);
	ite->setCondition(decorate(node->getCondition()));
	if (!isCondition(ite->getCondition()))
		throw DinoException("Expected a condition", EXT_GENERAL, node->getLine());
	ite->setThenBranch(decorate(node->getThenBranch()));
	ite->setElseBranch(decorate(node->getThenBranch()));
	return ite;
}

DST::ForLoop * Decorator::decorate(AST::ForLoop * node)
{
	auto loop = new DST::ForLoop(node);
	loop->setBegin(decorate(node->getBegin()));
	loop->setCondition(decorate(node->getCondition()));
	if (!isCondition(loop->getCondition()))
		throw DinoException("Expected a condition", EXT_GENERAL, node->getLine());
	loop->setIncrement(decorate(node->getIncrement()));
	loop->setStatement(decorate(node->getStatement()));
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
	if (node->getExpressionType() == ET_VARIABLE)
		return new DST::BasicType(node);
	return nullptr;
}

bool Decorator::isCondition(DST::Expression * node)
{
	return node && node->getType()->getExactType() == EXACT_BASIC
		&& dynamic_cast<DST::BasicType*>(node->getType())->getTypeId() == CONDITION_TYPE;
}
