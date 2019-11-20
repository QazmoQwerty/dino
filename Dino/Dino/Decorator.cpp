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
	case ET_VARIABLE_DECLARATION:
		return decorate(dynamic_cast<AST::VariableDeclaration*>(node));
	case ET_BINARY_OPERATION:
		return decorate(dynamic_cast<AST::BinaryOperation*>(node));
	case ET_LITERAL:
		return decorate(dynamic_cast<AST::Literal*>(node));
	}
	return NULL;
}

DST::Statement * Decorator::decorate(AST::Statement * node)
{
	switch (dynamic_cast<AST::Statement*>(node)->getStatementType())
	{
	
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

DST::BinaryOperation * Decorator::decorate(AST::BinaryOperation * node)
{
	auto bo = new DST::BinaryOperation(node, decorate(node->getLeft()), decorate(node->getRight()));

	// TODO - determine type
	bo->setType(bo->getLeft()->getType());	// (temporary)

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

DST::Type * Decorator::evalType(AST::Expression * node)
{
	if (node->getExpressionType() == ET_VARIABLE)
		return new DST::BasicType(node);
	return nullptr;
}
