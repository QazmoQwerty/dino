#include "Decorator.h"

unordered_map<unicode_string, DST::Type*, UnicodeHasherFunction> Decorator::_variables;
unordered_map<unicode_string, /* TypeDefinition* */ void*, UnicodeHasherFunction> Decorator::_types;

DST::Node *Decorator::decorate(AST::Node * node)
{
	if (node->isExpression())
	{
		switch (dynamic_cast<AST::Expression*>(node)->getExpressionType())
		{
			case ET_ASSIGNMENT:
				return decorate(dynamic_cast<AST::Assignment*>(node));
			case ET_VARIABLE:
				return decorate(dynamic_cast<AST::Variable*>(node));
		}
	}
	return NULL;
}

DST::Expression *Decorator::decorate(AST::Variable * node)
{
	if (_variables.count(node->getVarId()))
		return new DST::Variable(node, _variables[node->getVarId()]);

	else if (_types.count(node->getVarId()))
		return evalType(node);

	else 
		throw DinoException::DinoException("Identifier is undefined", EXT_GENERAL, node->getLine()); // TODO: add id to error.
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

DST::Type * Decorator::evalType(AST::Expression * node)
{
	if (node->getExpressionType() == ET_VARIABLE)
		return new DST::BasicType(node);
	return nullptr;
}
