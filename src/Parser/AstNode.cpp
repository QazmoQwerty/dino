/*
	This file implements the bulk of the AST classes' member functions.
*/
#include "AstNode.h"

int AST::_idCount = 0;

void AST::Function::addParameters(Expression * parameters)
{
	if (parameters == nullptr)
		return;
	if (!parameters->isExpression())
		throw ErrorReporter::report("expected a variable declaration", ERR_PARSER, parameters->getPosition());
	auto exp = dynamic_cast<Expression*>(parameters);
	switch (exp->getExpressionType())
	{
	case(ET_VARIABLE_DECLARATION):
		_parameters.push_back(dynamic_cast<VariableDeclaration*>(exp));
		break;
	case(ET_LIST):
		for (auto i : dynamic_cast<ExpressionList*>(exp)->getExpressions())
		{
			if (i->getExpressionType() == ET_VARIABLE_DECLARATION)
				_parameters.push_back(dynamic_cast<VariableDeclaration*>(i));
			else throw ErrorReporter::report("expected a variable declaration", ERR_PARSER, i->getPosition());
		}
		break;
	default:
		throw ErrorReporter::report("expected a variable declaration", ERR_PARSER, exp->getPosition());
		break;
	}
}

AST::InterfaceDeclaration::~InterfaceDeclaration() 
{
	delete _implements; _properties.clear(); _functions.clear(); 
}

string AST::InterfaceDeclaration::toString()
{
	return "<InterfaceDeclaration>\\n" + _name.to_string();
}

void AST::InterfaceDeclaration::addFunction(FunctionDeclaration * function)
{
	if (function && function->getContent())
		throw ErrorReporter::report("functions inside interfaces may not have a body", ERR_PARSER, function->getPosition());
	_functions.push_back(function);
}

void AST::FunctionDeclaration::setGenerics(AST::Expression* generics)
{
	if (generics)
	{
		if (generics->getExpressionType() == ET_LIST)
			for (auto i : ((AST::ExpressionList*)generics)->getExpressions())
				_generics.push_back(i);
		else _generics.push_back(generics);
	}
}

AST::TypeDeclaration::~TypeDeclaration() 
{ 
	delete _interfaces; _variableDeclarations.clear(); _functionDeclarations.clear(); _propertyDeclarations.clear();
}

string AST::TypeDeclaration::toString()
{
	return "<TypeDeclaration>\\n" + _name.to_string();
}

void AST::FunctionDeclaration::addParameter(Node * parameter)
{
	if (parameter == nullptr)
		return;
	if (!parameter->isExpression())
		throw ErrorReporter::report("expected a variable declaration", ERR_PARSER, parameter->getPosition());
	auto exp = dynamic_cast<Expression*>(parameter);
	switch (exp->getExpressionType())
	{
	case(ET_VARIABLE_DECLARATION):
		_parameters.push_back(dynamic_cast<VariableDeclaration*>(exp));
		break;
	case(ET_LIST):
		for (auto i : dynamic_cast<ExpressionList*>(exp)->getExpressions())
		{
			if (i->getExpressionType() == ET_VARIABLE_DECLARATION)
				_parameters.push_back(dynamic_cast<VariableDeclaration*>(i));
			else throw ErrorReporter::report("expected a variable declaration", ERR_PARSER, parameter->getPosition());
		}
		break;
	default:
		throw ErrorReporter::report("expected a variable declaration", ERR_PARSER, exp->getPosition());
	}
}

void AST::ExpressionList::addExpression(Expression * expression)
{
	if (expression && expression->getExpressionType() == ET_LIST)
		for (auto i : dynamic_cast<AST::ExpressionList*>(expression)->getExpressions())
			_expressions.push_back(i);
	else _expressions.push_back(expression);
}

