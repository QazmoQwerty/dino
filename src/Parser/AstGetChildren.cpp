/*
    Implementations for all the required getChildren() functions of all AST classes.
    These functions are used for the outputting of graphviz representations of the AST.
    This file is a bunch of boring boilerplate code which you have no reason to read :).

    TODO - (not important) clean up this code by making use of initializer lists.
*/
#include "AstNode.h"

vector<AST::Node*> AST::ForLoop::getChildren()
{
	vector<Node*> v;
	v.push_back(_begin);
	v.push_back(_condition);
	v.push_back(_increment);
	v.push_back(_statement);
	return v;
}

vector<AST::Node*> AST::ExpressionList::getChildren()
{
	vector<Node*> v;
	for (auto i : _expressions)
		v.push_back(i);
	return v;
}

vector<AST::Node*> AST::SwitchCase::getChildren()
{
	vector<Node*> v;
	v.push_back(_expression);
	v.push_back(_default);
	for (CaseClause i : _cases)
	{
		v.push_back(i._expression);
		v.push_back(i._statement);
	}
	return v;
}

vector<AST::Node*> AST::Identifier::getChildren()
{
	return vector<Node*>();
}

vector<AST::Node*> AST::ConstDeclaration::getChildren()
{
	return { _expression };
	/*vector<Node*> v;
	v.push_back(_expression);
	return v;*/
}

vector<AST::Node*> AST::TypeDeclaration::getChildren()
{
	vector<Node*> v;
	v.push_back(_interfaces);
	for (auto i : _variableDeclarations)
		v.push_back(i);
	for (auto i : _functionDeclarations)
		v.push_back(i);
	for (auto i : _propertyDeclarations)
		v.push_back(i);
	return v;
}

vector<AST::Node*> AST::NamespaceDeclaration::getChildren()
{
	vector<Node*> v;
	v.push_back(_statement);
	return v;
}

vector<AST::Node*> AST::PropertyDeclaration::getChildren()
{
	vector<Node*> v;
	v.push_back(_decl);
	v.push_back(_get != nullptr ? _get : NULL);
	//v.push_back(_set != nullptr ? _set : NULL);
	if (_set == nullptr)
		v.push_back(NULL);
	else v.push_back(_set);
	return v;
}

vector<AST::Node*> AST::VariableDeclaration::getChildren()
{
	vector<Node*> vec;
	vec.push_back(_type);
	return vec;
}

vector<AST::Node*> AST::FunctionDeclaration::getChildren()
{
	vector<Node*> v;
	v.push_back(_decl);
	for (auto i : _parameters)
		v.push_back(i);
	v.push_back(_content);
	return v;
}

vector<AST::Node*> AST::StatementBlock::getChildren()
{
	vector<Node*> v;
	for (auto i : _statements)
		v.push_back(i);
	return v;
}

vector<AST::Node*> AST::IfThenElse::getChildren()
{
	vector<Node*> v;
	v.push_back(_condition);
	v.push_back(_thenBranch);
	//v.push_back(nullptr);
	if (_elseBranch == nullptr)
		v.push_back(NULL);
	else v.push_back(_elseBranch);
	return v;
}

vector<AST::Node*> AST::WhileLoop::getChildren()
{
	vector<Node*> v;
	v.push_back(_condition);
	v.push_back(_statement);
	return v;
}

vector<AST::Node*> AST::Assignment::getChildren()
{
	vector<Node*> v;
	v.push_back(_left);
	v.push_back(_right);
	return v;
}

vector<AST::Node*> AST::Increment::getChildren()
{
	vector<Node*> v;
	v.push_back(_expression);
	return v;
}

vector<AST::Node*> AST::BinaryOperation::getChildren()
{
	vector<Node*> v;
	v.push_back(_left);
	v.push_back(_right);
	return v;
}

vector<AST::Node*> AST::UnaryOperation::getChildren()
{
	vector<Node*> v;
	v.push_back(_expression);
	return v;
}

vector<AST::Node*> AST::FunctionCall::getChildren()
{
	vector<Node*> v;
	v.push_back(_functionId);
	v.push_back(_arguments);
	return v;
}

vector<AST::Node*> AST::ConditionalExpression::getChildren()
{
	vector<Node*> v;
	v.push_back(_condition);
	v.push_back(_thenBranch);
	v.push_back(_elseBranch);
	return v;
}

vector<AST::Node*> AST::Function::getChildren()
{
	vector<Node*> v;
	v.push_back(_returnType);
	for (auto i : _parameters)
		v.push_back(i);
	v.push_back(_content);
	return v;
}

vector<AST::Node*> AST::InterfaceDeclaration::getChildren()
{
	vector<Node*> v;
	v.push_back(_implements);
	for (auto i : _properties)
		v.push_back(i);
	for (auto i : _functions)
		v.push_back(i);
	return v;
}

vector<AST::Node*> AST::UnaryOperationStatement::getChildren()
{
	vector<Node*> v;
	v.push_back(_expression);
	return v;
}